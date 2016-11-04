# This file provides examples how to increase or decrease the 
# verbosity of the output. 

# Set the output level of an individual algorithms
from Configurables import TbEventBuilder
TbEventBuilder().OutputLevel = DEBUG
# Options are VERBOSE, DEBUG, INFO, WARNING, ERROR

# Set the  global output level (all algorithms) 
MessageSvc().OutputLevel = DEBUG
