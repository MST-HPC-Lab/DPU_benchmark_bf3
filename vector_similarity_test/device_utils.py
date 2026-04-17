import os

def is_bluefield():
    """Detects if the current device is a BlueField DPU."""
    if os.path.exists('/sys/class/dmi/id/product_name'):
        with open('/sys/class/dmi/id/product_name', 'r') as f:
            if 'BlueField' in f.read():
                return True
    return False