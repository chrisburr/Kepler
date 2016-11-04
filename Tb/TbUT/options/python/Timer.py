__author__ = 'ja'

import time

def timer(method):

    def timed(*args, **kw):
        startTime = time.time()
        result = method(*args, **kw)
        endTime = time.time()

        print ('execution of %r  takes  %2.2f sec') % \
              (method.__name__, endTime-startTime)
        return result

    return timed