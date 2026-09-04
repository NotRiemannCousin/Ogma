# tools/lldb/MyProjectNameLldb.py

# LLDB Pretty-Printer Stub for MyProjectName
# Replace the contents of this file with actual formatters for your types.

# S -> as string
# I -> italic
CL_GRAY         = "\u00feC"
CL_CLEAN        = "\u00feE"
CL_WHITE_GRAY_I = "\u00feK"
CL_WHITE_I      = "\u00feN"
CL_YELLOW_S     = "\u00feS"
CL_WHITE        = "\u00feV"

def Style(message, color=CL_WHITE):
    return f"{color}{message}{CL_CLEAN}"

def MyTypeSummary(val, _dict):
    # Example of how to format a custom type
    # val is an SBValue
    return Style(f"MyType({val.GetChildMemberWithName('id').GetValue()})", CL_YELLOW_S)

class MyTypeSyntheticProvider:
    """MyProjectName::MyType synthetic provider."""
    def __init__(self, val, _dict):
        self._obj = val

    def update(self):
        return

    def num_children(self):
        return 1

    def get_child_at_index(self, index):
        if index == 0:
            return self._obj.GetChildMemberWithName("id")
        return None

    def has_children(self):
        return True

def __lldb_init_module(debugger, _dict):
    # Define the exact names of your classes here
    MY_TYPE_NAME = "MyProjectName::MyType"

    # Register the summary provider
    debugger.HandleCommand(f'type summary add -F MyProjectNameLldb.MyTypeSummary "{MY_TYPE_NAME}"')

    # Register the synthetic provider
    debugger.HandleCommand(f'type synthetic add "{MY_TYPE_NAME}" --python-class MyProjectNameLldb.MyTypeSyntheticProvider')