#! /usr/bin/env python
# Quex is  free software;  you can  redistribute it and/or  modify it  under the
# terms  of the  GNU Lesser  General  Public License  as published  by the  Free
# Software Foundation;  either version 2.1 of  the License, or  (at your option)
# any later version.
# 
# This software is  distributed in the hope that it will  be useful, but WITHOUT
# ANY WARRANTY; without even the  implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the  GNU Lesser General Public License for more
# details.
# 
# You should have received a copy of the GNU Lesser General Public License along
# with this  library; if not,  write to the  Free Software Foundation,  Inc., 59
# Temple Place, Suite 330, Boston, MA 02111-1307 USA
#
# (C) 2007 Frank-Rene Schaefer
#
################################################################################
# -*- python -*-
import os
import sys

QUEX_VERSION = '0.34.1'

try:
    QUEX_INSTALLATION_DIR = os.environ["QUEX_PATH"]
except:
    print "error: environment variable 'QUEX_PATH' is not defined."
    if os.name == "posix":
        print "error: your system is 'posix'."
        print "error: if you are using bash-shell, append the following line"
        print "error: to your '~/.bashrc' file:"
        print "error:"
        print "error: export QUEX_PATH=directory-where-quex-has-been-installed"

    elif os.name == "nt":
        print "error: Right click on [MyComputer]"
        print "error:                  -> [Properties]"
        print "error:                       -> Tab[Advanced]"
        print "error:                            -> [Environment Variables]"
        print "error: and from there it is obvious."

    else:
        print "error: for your system '%s' it is not known how to set environment" % os.name
        print "error: variables. if you find out, please, send an email to"
        print "error: <fschaef@users.sourceforge.net>"
    sys.exit(-1)

sys.path.insert(0, QUEX_INSTALLATION_DIR)
QUEX_TEMPLATE_DB_DIR = QUEX_INSTALLATION_DIR + "/quex/code_base"

def check():
    global QUEX_INSTALLATION_DIR

    # -- Try to acces the file 'quex-exe.py' in order to verify
    if os.access(QUEX_INSTALLATION_DIR + "/quex-exe.py", os.F_OK) == False:
        print "error: Environment variable 'QUEX_PATH' does not point to"
        print "error: a valid installation directory of quex."
        print "error: current setting of 'QUEX_PATH':"
        print "error:", QUEX_INSTALLATION_DIR
        sys.exit(-1)

    # -- Check for version 2.4 or higher
    if sys.version_info[0] < 2 or \
       (sys.version_info[0] == 2 and sys.version_info[1] < 4):
        print "error: Quex requires Python version 2.4 or higher. Detected version '%i.%i'." % \
              (sys.version_info[0], sys.version_info[1])
        print "error: Please, visit http://www.python.org and download an appropriate release."
        sys.exit(-1)

