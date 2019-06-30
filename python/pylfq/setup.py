from distutils.core import setup, Extension

setup(name='pylfq', version='0.1.0', include_dirs = '../../src',
      ext_modules=[Extension('pylfq', ['src/pylfq.c', '../../src/queue.c'])])