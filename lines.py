import re

def read_file(pathname, line_list):
    "read assembly lines into nested list"
    
    with open(pathname, 'r') as f:
        lno = 1
        for line in f:
            expanded_line = line.expandtabs().rstrip()
            # split the line into "words"
#             word_list = expanded_line.split()
            word_list = re.findall("(?:\".*?\"|\S)+", expanded_line)

            # If the line begins with whitespace then prepend an empty string to the word list
            if expanded_line.startswith((' ')):
                word_list.insert(0, '')

            line_list.append([pathname, lno, expanded_line, word_list])
            lno += 1
            
            if len(word_list) >= 3:
                if word_list[1] == 'include':
                    read_file(word_list[2].strip('"'), line_list)
    return

def print_lines(line_list):
    "print a formatted assembly listing"
    
    for line in line_list:
         print(line[0],       # pathname
               ":", line[1],  # line number
               ' ', line[2],  # expanded line
               sep = '')      # no extra spaces
    
all_lines = []
read_file("newtick.a", all_lines)
print (all_lines)
 