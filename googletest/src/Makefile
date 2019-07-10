TOP=../..
include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE

#----------------------------------------------------
#  Optimization of db files using dbst (DEFAULT: NO)
#DB_OPT = YES

#----------------------------------------------------

USR_CPPFLAGS += -I$(GTEST)/googletest/include -I$(GTEST)/googletest

LIBRARY = gtest gtest_main
gtest_SRCS =  gtest-all.cc
gtest_main_SRCS =  gtest_main.cc
gtest_main_LIBS += gtest
LIBTYPE:=SHARED

#----------------------------------------------------
# If <anyname>.db template is not named <anyname>*.template add
# <anyname>_template = <templatename>

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

ifeq ($(STATIC_BUILD),NO)
$(LIB_PREFIX)gtest$(LIB_SUFFIX) : $(LIB_PREFIX)%$(LIB_SUFFIX):
	@$(RM) $@
	$(ARCMD)
ifneq ($(strip $(RANLIB)),)
	$(RANLIB) $@
endif
$(LIB_PREFIX)gtest_main$(LIB_SUFFIX) : $(LIB_PREFIX)%$(LIB_SUFFIX):
	@$(RM) $@
	$(ARCMD)
ifneq ($(strip $(RANLIB)),)
	$(RANLIB) $@
endif
endif
