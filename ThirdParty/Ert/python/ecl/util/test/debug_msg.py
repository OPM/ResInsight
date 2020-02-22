import inspect

def debug_msg(msg):
    record = inspect.stack()[1]
    frame = record[0]
    info = inspect.getframeinfo( frame )
    return "FILE: %s  LINE: %s  Msg: %s" % (info.filename, info.lineno, msg)
