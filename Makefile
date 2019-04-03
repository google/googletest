TOP = .
include $(TOP)/configure/CONFIG
# Directories to build, any order
DIRS += configure
DIRS += googletest

# All dirs except configure depend on configure
$(foreach dir, $(filter-out configure, $(DIRS)), \
    $(eval $(dir)_DEPEND_DIRS += configure))
	
$(foreach dir, $(filter %Top, $(DIRS)), \
    $(eval $(dir)_DEPEND_DIRS += $(filter %googletest, $(DIRS))))
	
include $(TOP)/configure/RULES_TOP

