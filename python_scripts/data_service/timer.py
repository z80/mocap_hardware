
import time

class Timer:
    def __init__(self, interval):
        self.interval = interval  # Time interval in seconds
        self.last_time = time.time()  # Store the current time during initialization

    def __call__(self):
        import pdb
        pdb.set_trace()
        current_time = time.time()  # Get the current time
        delta = current_time - self.last_time
        if current_time - self.last_time < self.interval:
            return False

        print( "delta", delta )
        # Adjust the last_time snapshot to match intervals
        self.last_time += self.interval
        # Account for potential drift
        if current_time - self.last_time >= self.interval:
            self.last_time = current_time
        return True


if __name__ == '__main__':
    import pdb
    pdb.set_trace()
    timer = Timer( 2.0 )

    while (True):
        ret = timer()
        print( ret )




