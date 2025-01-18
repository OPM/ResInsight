# test_example.py

import sys
sys.path.append('../../build/native-python')  # Replace with the actual path

try:
    import msjmodule
except ImportError as e:
    print("Error importing module:", e)


try:
    # Test the `add` function
    result = msjmodule.add(3, 5)
    print(f"Result of add(3, 5): {result}")  # Expected output: 8

    # Check edge cases
    print(f"add(0, 0): {msjmodule.add(0, 0)}")      # Expected output: 0

    #msjmodule.init_singletons()
    #print(f"add(-1, 1): {msjmodule.add(-1, 1)}")    # Expected output: 0

    text = msjmodule.octave_path()
    print(f"octave_path(): {text}")            # Expected output: "Singletons

    print(f"add(100, 200): {msjmodule.add(100, 200)}")  # Expected output: 300
except RuntimeError as e:
    print("Error executing module:", e)
