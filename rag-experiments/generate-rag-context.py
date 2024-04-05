#!/usr/bin/python3
import re, json, sys
import pypdfium2 as pdfium

chunk_size = 1000
chunk_sentences_overlap = 1
sentence_min = 10

'''
Check if a line in the PDF matches a heading in the table of
contents. The heading is a string that may contain a page number, so
we need to filter out the page number from the line before checking if
it matches the heading. Note that for now the regex matching here
is very specific to the OpenMP spec PDF.
'''
def heading_match(heading, line):
    if heading == "Index": return line.strip() == "Index"
    filtered_line = re.search(r'[\d]+ ((?:[\d]|A|B)[\d\.]* [A-Za-z\- ]+)', line)
    if filtered_line:
        print("Filtered line: ", filtered_line.group(1))
        return filtered_line.group(1) in heading
    return False

'''
Get all the text under each table of contents entry in the PDF.
Returns a dictionary with the section title as the key and all
the text between that section title and the next section title
as the value.
'''
def split_by_section(pdf, toc):
    context = {}
    current_section = toc[0].title
    searching_for = toc[1].title
    searching_toc_index = 1
    buffer = ''
    for i, page in enumerate(pdf):
        if i >= toc[0].page_index:
            for line in page.get_textpage().get_text_range().splitlines():
                if searching_for and heading_match(searching_for, line):
                    print("Found ", searching_for, "expected page ", toc[searching_toc_index].page_index, "got page ", i)
                    print("Line matched: ", line)
                    context[current_section] = buffer
                    buffer = ''
                    current_section = searching_for
                    searching_toc_index += 1
                    if searching_toc_index < len(toc):
                        searching_for = toc[searching_toc_index].title
                        print("Searching for ", searching_for)
                    else:
                        searching_for = None
                else:
                    buffer += line + ' \n'
    context[current_section] = buffer
    return context

'''
Take the context dictionary and split each section into chunks of
about size chunk_size, with some overlapping sentences determined by
chunk_sentences_overlapping. Does not split sentences.
'''
def explode_by_chunk(context):
    exploded_context = {}
    for section, text in context.items():
        exploded_context[section] = []
        sentences = re.split(r'(?<=[.]) +', text)
        #sentences = re.split(r' +\\n', text)
        chunk = ''
        for i in range(len(sentences)):
            sentence = sentences[i]
            if len(chunk) + len(sentence) > chunk_size:
                exploded_context[section].append(chunk)
                if i - chunk_sentences_overlap < 0:
                    chunk = ''.join(sentences[0:i]) + sentence
                else:
                    chunk = ''.join(sentences[i - chunk_sentences_overlap:i]) + sentence
            else:
                chunk += sentence
        exploded_context[section].append(chunk)
    return exploded_context

'''
Take the context dictionary and split each section into
sentences, ignoring any sentences shorter than
sentence_min. Note this function is NOT used by default,
just provided as an option.
'''
def explode_by_sentence(context):
    exploded_context = {}
    for section, text in context.items():
        exploded_context[section] = re.split(r'(?<=[.]) +', text)
        exploded_context[section] = [sentence for sentence in exploded_context[section] if len(sentence) > sentence_min]
    return exploded_context

'''
Take a context as a dictionary with section titles as keys and
text as values, and format it as a list of dictionaries with
"section_title" and "text" keys. This is the format expected by
HuggingFace datasets. Also removes line numbers from the text.
'''
def huggingface_format(context):
    hf_context = []
    for section, text in context.items():
        for chunk in text:
            chunk_text = re.sub(r'\n[\d]+', '\n', chunk)
            hf_context.append({"section_title": section, "chunk": chunk_text})
    return hf_context

# --- Main ---

# Get path to spec PDF
file_path = sys.argv[1]
output_path = sys.argv[2]

# Read the PDF
print(f"Reading PDF: {file_path}")
pdf = pdfium.PdfDocument(file_path)
version = pdf.get_version()
print(f"PDF version: {version}")
n_pages = len(pdf)
print(f"Number of pages: {n_pages}")
toc = list(pdf.get_toc())

context = split_by_section(pdf, toc)
context = explode_by_chunk(context)

# Check for unmatched headings
for heading in toc:
    if heading.title not in context:
        print(f"WARNING! Unmatched heading: {heading.title}")

# Section plans:
# always: 1, 3, 5, 10, 11

# Drop some sections that won't be useful
sections_to_keep = [r"1", r"3", r"5", r"10", r"11"]
sections_to_remove = []
regex_base = r"\A{n}[\. ][\. \w]+"
context_copy = context.copy()
for section in context_copy:
    will_keep = False
    for n in sections_to_keep:
        if re.search(regex_base.format(n=n), section):
            will_remove = False
            for m in sections_to_remove:
                if re.search(regex_base.format(n=m), section):
                    will_remove = True
                    break
            #print(f"Keeping section {section}, matches {n}")
            will_keep = True
            break
    if not will_keep: del context[section]


context = huggingface_format(context)

# Write the context to a file
print(f"Writing context to file")
with open(output_path, 'w') as f:
    for row in context:
        f.write(json.dumps(row))
        f.write('\n')
    #json.dump(context, f, indent=4)

print(f"Context written to {output_path}")

# Below: some code to get the text from a specific page
# for testing regexes
'''
test_page = 660
print(f"Reading page {test_page}")
page = pdf[test_page]
page_text = page.get_textpage().get_text_range()
print(page_text)
'''
