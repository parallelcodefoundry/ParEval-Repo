#define TESTING
#include "train_gpt2_fp32.cu"

// poor man's tensor checker
int check_tensor(float *a, float *b, int n, const char* label) {
    int print_upto = 5;
    int ok = 1;
    printf("%s\n", label);
    for (int i = 0; i < n; i++) {
        if (fabsf(a[i] - b[i]) <= 1e-2) {
            if (i < print_upto) { printf("OK "); }
        } else {
            if (i < print_upto) { printf("NOT OK "); }
            ok = 0;
        }
        if (i < print_upto) { printf("%f %f\n", a[i], b[i]); }
    }
    // print the final result
    if (ok) {
        printf("TENSOR OK\n");
    } else {
        printf("TENSOR NOT OK\n");
    }
    return ok;
}

int main(int argc, char *argv[]) {

    // set up the device
    int deviceIdx = 0;
    // No need to set device in OpenMP offload
    // cudaCheck(cudaSetDevice(deviceIdx));
    cudaDeviceProp deviceProp;
    // No need to get device properties in OpenMP offload
    // cudaGetDeviceProperties(&deviceProp, deviceIdx);
    printf("[System]\n");
    // No need to print device name in OpenMP offload
    // printf("Device %d: %s\n", deviceIdx, deviceProp.name);

    // setup cuBLAS and cuBLASLt
    // No need for cuBLAS in OpenMP offload
    // cublasCheck(cublasCreate(&cublas_handle));
    // TF32 precision is equivalent to torch.set_float32_matmul_precision('high')
    int enable_tf32 = deviceProp.major >= 8 ? 1 : 0;
    enable_tf32 = 0; // NOTE: disable TF32 for testing!!!
    printf("enable_tf32: %d\n", enable_tf32);
    // No need to set compute type in OpenMP offload
    // cublas_compute_type = enable_tf32 ? CUBLAS_COMPUTE_32F_FAST_TF32 : CUBLAS_COMPUTE_32F;
    // cublasMath_t cublas_math_mode = enable_tf32 ? CUBLAS_TF32_TENSOR_OP_MATH : CUBLAS_DEFAULT_MATH;
    // No need to set math mode in OpenMP offload
    // cublasCheck(cublasSetMathMode(cublas_handle, cublas_math_mode));

    // build the GPT-2 model from a checkpoint
    GPT2 model;
    gpt2_build_from_checkpoint(&model, "gpt2_124M.bin");

    // int C = model.config.channels;
    int V = model.config.vocab_size;
    int Vp = model.config.padded_vocab_size;
    int maxT = model.config.max_seq_len;
    // int L = model.config.num_layers;

    // load additional information that we will use for debugging and error checking
    FILE *state_file = fopenCheck("gpt2_124M_debug_state.bin", "rb");
    int state_header[256];
    freadCheck(state_header, sizeof(int), 256, state_file);
    if (state_header[0] != 20240327) { printf("Bad magic state file\n"); exit(EXIT_FAILURE); }
    if (state_header[1] != 2) {
        fprintf(stderr, "Bad version in state file\n");
        fprintf(stderr, "---> HINT: try to re-run `python train_gpt2.py`\n");
        exit(EXIT_FAILURE);
    }
    int B = state_header[2]; // batch size, e.g. 4
    int T = state_header[3]; // time / sequence length (e.g. 64, up to maxT)
    assert(0 <= T && T <= maxT);
    printf("[State]\n");
    printf("batch_size: %d\n", B);
    printf("seq_len: %d\n", T);

    ParameterTensors expected_grads; // will be read from file (from PyTorch)
    ParameterTensors calculated_grads; // will be calculated by us
    float* expected_grads_memory = malloc_and_point_parameters(&expected_grads, model.param_sizes, 0);
    float* calculated_grads_memory = malloc_and_point_parameters(&calculated_grads, model.param_sizes, 0);

    // inputs and expected outputs, only used for error checking
    int* x = (int*)mallocCheck(B * T * sizeof(int));
    int* y = (int*)mallocCheck(B * T * sizeof(int));
    float* expected_logits = (float*) mallocCheck(B * T * V * sizeof(float));
    float* expected_loss = (float*) mallocCheck(1 * sizeof(float));

    // read reference information from Python
    freadCheck(x, sizeof(int), B*T, state_file);
    freadCheck(y, sizeof(int), B*T, state_file);
    freadCheck(expected_logits, sizeof(float), B*T*V, state_file);
    freadCheck(expected_loss, sizeof(float), 1, state_file);
    freadCheck(expected_grads_memory, sizeof(float), model.num_parameters, state_file);
    fcloseCheck(state_file);

    // overall OK signal for the test
    int allok = 1;

    // First, do target-free forward pass to validate logits
    // Allocate device memory for activations
    size_t act_sizes[NUM_ACTIVATION_TENSORS];
    fill_in_activation_sizes(act_sizes, B, T, model.config);
    size_t num_activations = 0;
    for (size_t i = 0; i < NUM_ACTIVATION_TENSORS; i++) {
        num_activations += act_sizes[i];
    }
    float* acts_memory;
    cudaMalloc((void**)&acts_memory, num_activations * sizeof(float));
    ActivationTensors acts;
    float** ptrs[] = {
        &acts.encoded, &acts.ln1, &acts.ln1_mean, &acts.ln1_rstd, &acts.atty,
        &acts.att, &acts.attproj, &acts.residual2, &acts.ln2, &acts.ln2_mean,
        &acts.ln2_rstd, &acts.fch, &acts.fch_gelu, &acts.fcproj, &acts.residual3, &acts.lnf,
        &acts.lnf_mean, &acts.lnf_rstd, &acts.losses, &acts.qkvr, &acts.output
    };
    float* acts_memory_iterator = acts_memory;
    for (size_t i = 0; i < NUM_ACTIVATION_TENSORS; i++) {
        *(ptrs[i]) = acts_memory_iterator;
        acts_memory_iterator += act_sizes[i];
    }
    // Allocate device memory for inputs
    int* inputs;
    cudaMalloc((void**)&inputs, B * T * sizeof(int));
    // Allocate device memory for targets
    int* targets;
    cudaMalloc((void**)&targets, B * T * sizeof(int));
    // Copy input data to device
    cudaMemcpy(inputs, x, B * T * sizeof(int), cudaMemcpyHostToDevice);
    #pragma omp target data map(to: model.params, acts) map(from: acts) map(tofrom: inputs)
    {
        gpt2_forward(&model, inputs, targets, B, T);
    }
    // at this point, target should be equal to expected_logits, let's compare
    // copy logits to CPU so we can compare them
    float* logits_cpu = (float*)mallocCheck(B * T * Vp * sizeof(float));
    cudaMemcpy(logits_cpu, acts.output, B * T * Vp * sizeof(float), cudaMemcpyDeviceToHost);

    // compare the output logits from the forward pass
    // also careful that we don't access and compare the padded columns of logits
    int logits_ok = 1;
    float max_diff = 0.0f;
    for (int bt = 0; bt < B*T; bt++) {
        for (int v = 0; v < V; v++) {
            int i = bt * Vp + v; // linearized index
            if (i < 10) {
                printf("%f, %f\n", expected_logits[i], logits_cpu[i]);
            }
            float diff = fabsf(expected_logits[bt*V + v] - logits_cpu[i]);
            max_diff = fmaxf(max_diff, diff);
            if (diff >= 1e-2f) {
                printf("MISMATCH AT INDEX %d,%d: ", bt, v);
                printf("%f %f\n", expected_logits[bt*V + v], logits_cpu[i]);
                logits_ok = 0;
                bt = B*T; // to break out of both loops
                break;
            }
        }
    }
    allok = allok && logits_ok;
    if(!logits_ok) { printf("NOT "); }
    printf("OK (LOGITS)\n");

    // Allocate device memory for gradients of activations
    size_t bw_act_sizes[NUM_BACKWARD_TENSORS];
    GPT2Config cfg = model.config;
    cfg.num_layers = 1;
    fill_in_grad_act_sizes(bw_act_sizes, B, T, cfg);
    size_t num_grad_acts = 0;
    for (int i = 0; i < NUM_BACKWARD_TENSORS; i++) {
        num_grad_acts += bw_act_sizes[i];
    }
    float* grads_acts_memory;
    cudaMalloc((void**)&grads_acts_memory, num_grad_acts * sizeof(float));
    GradActTensors grads_acts;
    float** ptrs_grad_acts[] = {
        &grads_acts.bt4c, &grads_acts.preatt, &grads_acts.residual3
    };
    float* grads_acts_memory_iterator = grads_acts_memory;
    for (size_t i = 0; i < NUM_BACKWARD_TENSORS; i++) {
        *(ptrs_grad_acts[i]) = grads_acts_memory_iterator;
        grads_acts_memory_iterator += bw_act_sizes[i];
    }
    // Allocate device memory for gradients of parameters
    float* grads_memory = malloc_and_point_parameters(&model.grads, model.param_sizes, 1);
    // let's do 10 training iterations, following the pytorch code
    float losses[10];
    for (int step = 0; step < 10; step++) {
        struct timespec start, end;
        clock_gettime(CLOCK_MONOTONIC, &start);
        // Copy target data to device
        cudaMemcpy(targets, y, B * T * sizeof(int), cudaMemcpyHostToDevice);
        // Zero out gradients
        cudaMemset(grads_acts_memory, 0, num_grad_acts * sizeof(float));
        cudaMemset(grads_memory, 0, model.num_parameters * sizeof(float));
        #pragma omp target data map(to: model.params, acts, grads) map(from: acts, grads) map(tofrom: inputs, targets)
        {
            gpt2_forward(&model, inputs, targets, B, T);
            gpt2_backward(&model);
        }
        clock_gettime(CLOCK_MONOTONIC, &end);
        double time_elapsed_s = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

        if (step == 0) {
            // error checking at step 0 for reference activations
            free(logits_cpu);

            // compare the achieved loss
            if (fabsf(model.mean_loss - *expected_loss) >= 1e-2) {
                printf("LOSS MISMATCH: %f %f\n", model.mean_loss, *expected_loss);
                allok = 0;
            } else {
                printf("LOSS OK: %f %f\n", model.mean_loss, *expected_loss);
            }

            // compare the gradients ona the parameters all at once
            cudaMemcpy(calculated_grads_memory, model.grads_memory, model.num_parameters * sizeof(float), cudaMemcpyDeviceToHost);
            check_tensor(calculated_grads_memory, expected_grads_memory, model.num_parameters, "grads");
        }

        // Update parameters on the device
        #pragma omp target data map(tofrom: model.params)
        {
            gpt2_update(&model, 1e-4f, 0.9f, 0.999f, 1e-8f, 0.01f, step+1);
        }

        // print the timing information at the end
        printf("step %d: loss %f (took %f ms)\n", step, model.mean_loss, time_elapsed_s * 1000);
        losses[step] = model.mean_loss;
    }

    // expected losses are as follows, from Python
    float expected_losses[10] = {
        5.270007133483887f,
        4.059706687927246f,
        3.3751230239868164f,
        2.8007826805114746f,
        2.315382242202759f,
        1.8490285873413086f,
        1.3946564197540283f,
        0.9991465210914612f,
        0.6240804195404053f,
        0.37651097774505615f
    };

    // compare
    for (int i = 0; i < 10; i++) {
        if (fabsf(losses[i] - expected_losses[i]) >= 1e-2) {
            printf("LOSS MISMATCH AT STEP %d: %f %f\n", i, losses[i], expected_losses[i]);
            allok = 0;
        } else {
            printf("loss ok at step %d: %f %f\n", i, losses[i], expected_losses[i]);
        }
    }

    // final approval
    printf("overall okay: %d\n", allok);

    // free everything
    free(x);
    free(y);
    free(expected_logits);
    free(expected_loss);
    free(expected_grads_memory);
    free(calculated_grads_memory);
    cudaFree(acts_memory);
    cudaFree(grads_acts_memory);
    cudaFree(grads_memory);
    cudaFree(inputs);
    cudaFree(targets);
    // No need to destroy cuBLAS in OpenMP offload
    // cublasCheck(cublasDestroy(cublas_handle));

    return 0;
}