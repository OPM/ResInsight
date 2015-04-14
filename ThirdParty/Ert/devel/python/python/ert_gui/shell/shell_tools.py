import fnmatch
import os
import shlex


def autoCompleteList(text, items):
    if not text:
        completions = items
    else:
        completions = [item for item in items if item.lower().startswith(text.lower())]
    return completions

def containsAny(string, chars):
    return True in [char in string for char in chars]

def findRightMostSeparator(text, separators):
    max_pos = 0

    for separator in separators:
        pos = text.rfind(separator)
        max_pos = max(pos, max_pos)

    return max_pos

def autoCompleteListWithSeparator(text, items, separators=":,@"):
    if containsAny(text, separators):
        auto_complete_list = autoCompleteList(text, items)
        separator_pos = findRightMostSeparator(text, separators)
        auto_complete_list = [item[separator_pos + 1:] for item in auto_complete_list]
    else:
        auto_complete_list = autoCompleteList(text, items)

    return auto_complete_list


def createParameterizedHelpFunction(parameters, help_message):
    def helpFunction(self):
        return parameters, help_message

    return helpFunction


def pathify(head, tail):
    path = os.path.join(head, tail)
    if os.path.isdir(path):
        return "%s/" % tail
    return tail


def getPossibleFilenameCompletions(text, separators="-"):
    head, tail = os.path.split(text.strip())
    if head == "":  # no head
        head = "."
    files = os.listdir(head)

    separator_pos = 0
    if containsAny(tail, separators):
        separator_pos = findRightMostSeparator(tail, separators) + 1

    return [pathify(head, f)[separator_pos:] for f in files if f.startswith(tail)]


def extractFullArgument(line, endidx):
    newstart = line.rfind(" ", 0, endidx)
    return line[newstart:endidx].strip()


def matchItems(line, items):
    patterns = shlex.split(line)

    result_items = set()

    for pattern in patterns:
        pattern_matches = set()
        for item in items:
            if fnmatch.fnmatch(item.lower(), pattern.lower()): # case-insensitive matching
                pattern_matches.add(item)

        if len(pattern_matches) == 0:
            print("Error: Name/Pattern '%s' does not match anything." % pattern)
        else:
            result_items = result_items | pattern_matches

    return result_items