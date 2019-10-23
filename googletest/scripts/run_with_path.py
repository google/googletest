#!/usr/bin/env python
#
# Copyright 2010 Google Inc. All Rights Reserved.

"""Runs program specified in the command line with the substituted PATH.

   This script is needed for to support building under Pulse which is unable
   to override the existing PATH variable.
"""

import os
import subprocess
import sys

SUBST_PATH_ENV_VAR_NAME = "SUBST_PATH"

def main():
  if SUBST_PATH_ENV_VAR_NAME in os.environ:
    os.environ["PATH"] = os.environ[SUBST_PATH_ENV_VAR_NAME]

  exit_code = subprocess.Popen(sys.argv[1:]).wait()

  # exit_code is negative (-signal) if the process has been terminated by
  # a signal. Returning negative exit code is not portable and so we return
  # 100 instead.
  if exit_code < 0:
    exit_code = 100

  sys.exit(exit_code)

if __name__ == "__main__":
  main()
