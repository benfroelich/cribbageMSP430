################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
fram-utilities/ctpl/ctpl.obj: ../fram-utilities/ctpl/ctpl.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"/Applications/ti/ccsv7/tools/compiler/ti-cgt-msp430_16.9.0.LTS/bin/cl430" -vmspx --data_model=restricted --use_hw_mpy=F5 --include_path="/Applications/ti/ccsv7/ccs_base/msp430/include" --include_path="/Applications/ti/ccsv7/tools/compiler/ti-cgt-msp430_16.9.0.LTS/include" --advice:power="all" --advice:hw_config="1" --define=__MSP430FR5969__ --define=CTPL_STACK_SIZE=160 --define=_MPU_ENABLE -g --c99 --c++03 --relaxed_ansi --diag_warning=225 --diag_wrap=off --display_error_number --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU40 --single_inline --preproc_with_compile --preproc_dependency="fram-utilities/ctpl/ctpl.d" --obj_directory="fram-utilities/ctpl" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

fram-utilities/ctpl/ctpl_low_level.obj: ../fram-utilities/ctpl/ctpl_low_level.asm $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"/Applications/ti/ccsv7/tools/compiler/ti-cgt-msp430_16.9.0.LTS/bin/cl430" -vmspx --data_model=restricted --use_hw_mpy=F5 --include_path="/Applications/ti/ccsv7/ccs_base/msp430/include" --include_path="/Users/benny/git/cribbage/cribbage_LED/fram-utilities/ctpl" --include_path="/Applications/ti/ccsv7/tools/compiler/ti-cgt-msp430_16.9.0.LTS/include" --advice:power="all" --advice:hw_config="1" --define=__MSP430FR5969__ --define=CTPL_STACK_SIZE=160 --define=_MPU_ENABLE -g --c99 --c++03 --relaxed_ansi --diag_warning=225 --diag_wrap=off --display_error_number --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU40 --single_inline --preproc_with_compile --preproc_dependency="fram-utilities/ctpl/ctpl_low_level.d" --obj_directory="fram-utilities/ctpl" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

fram-utilities/ctpl/ctpl_low_level_macros.obj: ../fram-utilities/ctpl/ctpl_low_level_macros.asm $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"/Applications/ti/ccsv7/tools/compiler/ti-cgt-msp430_16.9.0.LTS/bin/cl430" -vmspx --data_model=restricted --use_hw_mpy=F5 --include_path="/Applications/ti/ccsv7/ccs_base/msp430/include" --include_path="/Applications/ti/ccsv7/tools/compiler/ti-cgt-msp430_16.9.0.LTS/include" --advice:power="all" --advice:hw_config="1" --define=__MSP430FR5969__ --define=CTPL_STACK_SIZE=160 --define=_MPU_ENABLE -g --c99 --c++03 --relaxed_ansi --diag_warning=225 --diag_wrap=off --display_error_number --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU40 --single_inline --preproc_with_compile --preproc_dependency="fram-utilities/ctpl/ctpl_low_level_macros.d" --obj_directory="fram-utilities/ctpl" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


