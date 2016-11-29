import os
from ert_gui.shell.libshell import containsAny, extractFullArgument, findRightMostSeparator


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


def pathCompleter(model, text, line, begidx, endidx):
    argument = extractFullArgument(line, endidx)
    return getPossibleFilenameCompletions(argument)