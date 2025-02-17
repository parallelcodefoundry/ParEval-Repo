#!/bin/bash

ollama serve &
sleep 10
export OLLAMA_MODELS=$SCRATCH/ollama
$*
pkill ollama
