# mem_utils.py
import os
import time
import threading
import atexit

try:
    import psutil
    def _rss_mb():
        return psutil.Process(os.getpid()).memory_info().rss / (1024.0 * 1024.0)
except Exception:
    import resource
    def _rss_mb():
        # ru_maxrss is in kilobytes on Linux
        return resource.getrusage(resource.RUSAGE_SELF).ru_maxrss / 1024.0

def _now():
    return time.strftime("%Y-%m-%d %H:%M:%S")

def ensure_dir_for(path):
    d = os.path.dirname(path)
    if d:
        os.makedirs(d, exist_ok=True)

class MemorySampler(threading.Thread):
    def __init__(self, outpath, interval=1.0):
        super().__init__(daemon=True)
        self.outpath = outpath
        self.interval = interval
        self._stop = threading.Event()
        ensure_dir_for(outpath)
        existed = os.path.exists(self.outpath)
        with open(self.outpath, "a") as f:
            if not existed:
                f.write("timestamp,stage,rss_mb\n")

    def run(self):
        while not self._stop.is_set():
            try:
                with open(self.outpath, "a") as f:
                    f.write(f"{_now()},sample,{_rss_mb():.3f}\n")
            except Exception:
                pass
            time.sleep(self.interval)

    def stop(self):
        self._stop.set()
        self.join()

def get_peak_from_proc():
    res = {}
    try:
        with open("/proc/self/status") as f:
            for line in f:
                if line.startswith("VmRSS:"):
                    parts = line.split()
                    if len(parts) >= 2:
                        res["VmRSS_kB"] = int(parts[1])
                if line.startswith("VmPeak:"):
                    parts = line.split()
                    if len(parts) >= 2:
                        res["VmPeak_kB"] = int(parts[1])
    except Exception:
        pass
    return res

class MemoryMonitor:
    """
    Context manager that starts sampling on enter and stops on exit.
    Creates results/<role>_memory_<timestamp>.csv and a peak file on stop.
    """
    def __init__(self, role='run', outpath=None, interval=1.0, results_dir='results'):
        ts = int(time.time())
        if outpath is None:
            outpath = f"{results_dir}/{role}_memory_{ts}.csv"
        ensure_dir_for(outpath)
        self.outpath = outpath
        self.interval = interval
        self.sampler = MemorySampler(self.outpath, interval=self.interval)
        self._started = False
        atexit.register(self._atexit_cleanup)

    def start(self):
        if not self._started:
            self.sampler.start()
            with open(self.outpath, "a") as f:
                f.write(f"{_now()},monitor_started,{_rss_mb():.3f}\n")
            self._started = True

    def log(self, stage):
        ensure_dir_for(self.outpath)
        try:
            with open(self.outpath, "a") as f:
                f.write(f"{_now()},{stage},{_rss_mb():.3f}\n")
        except Exception:
            pass

    def stop(self):
        if self._started:
            try:
                self.sampler.stop()
            except Exception:
                pass
            self.log("monitor_stopped")
            peak = get_peak_from_proc()
            if peak:
                peakfile = self.outpath.replace('.csv', '_peak.txt')
                try:
                    with open(peakfile, "w") as f:
                        f.write(str(peak) + "\n")
                except Exception:
                    pass
            self._started = False

    def _atexit_cleanup(self):
        try:
            self.stop()
        except Exception:
            pass

    def __enter__(self):
        self.start()
        return self

    def __exit__(self, exc_type, exc, tb):
        self.stop()
        return False
