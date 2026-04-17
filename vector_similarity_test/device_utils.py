import os

# NOTE: this function is directly AI-coded, for convenience
def is_bluefield():
    """Detects if the current device is a BlueField DPU."""
    # Method 1: Check Mellanox platform file
    if os.path.exists('/etc/mellanox/mlnx-platform'):
        with open('/etc/mellanox/mlnx-platform', 'r') as f:
            if 'BlueField' in f.read():
                return True
                
    # Method 2: Check for specific device tree entries
    if os.path.exists('/sys/firmware/devicetree/base/model'):
        with open('/sys/firmware/devicetree/base/model', 'r') as f:
            if 'BlueField' in f.read():
                return True
                
    return False