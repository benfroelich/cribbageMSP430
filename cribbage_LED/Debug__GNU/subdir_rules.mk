################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
8117.memsafety.o: /Users/benny/git/cribbage/cribbage_LED/8117.memsafety.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: GNU Compiler'
	"/Users/benny/ti/tirex-content/energia-0101E0017/hardware/tools/msp430/bin/msp430-gcc" -c -mmcu=msp430fr5969 -I"/Applications/ti/ccsv7/ccs_base/msp430/include_gcc" -I"/Users/benny/ti/tirex-content/energia-0101E0017/hardware/tools/msp430/msp430/include" -O0 -Os -g -gdwarf-3 -gstrict-dwarf -Wall -mlarge -mhwmult=f5series -mcode-region=none -mdata-region=none -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o"$@" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

%.o: ../%.cpp $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: GNU Compiler'
	"/Users/benny/ti/tirex-content/energia-0101E0017/hardware/tools/msp430/bin/msp430-gcc" -c -mmcu=msp430fr5969 -I"/Applications/ti/ccsv7/ccs_base/msp430/include_gcc" -I"/Users/benny/ti/tirex-content/energia-0101E0017/hardware/tools/msp430/msp430/include" -O0 -Os -g -gdwarf-3 -gstrict-dwarf -Wall -mlarge -mhwmult=f5series -mcode-region=none -mdata-region=none -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o"$@" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

%.o: ../%.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: GNU Compiler'
	"/Users/benny/ti/tirex-content/energia-0101E0017/hardware/tools/msp430/bin/msp430-gcc" -c -mmcu=msp430fr5969 -I"/Applications/ti/ccsv7/ccs_base/msp430/include_gcc" -I"/Users/benny/ti/tirex-content/energia-0101E0017/hardware/tools/msp430/msp430/include" -O0 -Os -g -gdwarf-3 -gstrict-dwarf -Wall -mlarge -mhwmult=f5series -mcode-region=none -mdata-region=none -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o"$@" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


