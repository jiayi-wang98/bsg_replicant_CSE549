# vector-size, warm-cache
#1530*2554*3
#3.9MB*3=11.7MB
TESTS += $(call test-name,32,16,1530,2554,no)
TESTS += $(call test-name,32,16,1530,2554,yes)
TESTS += $(call test-name,32,16,64,64,no)
TESTS += $(call test-name,32,16,64,64,yes)
TESTS += $(call test-name,8,4,64,64,no)
TESTS += $(call test-name,8,4,64,64,yes)
