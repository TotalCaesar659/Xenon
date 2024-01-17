﻿#include <bitset>
#include <iostream>
#include <fstream>
#include <thread>

#include "Xenon/Core/XCPU/XCPU.h"
#include "Xenon/Core/XCPU/Interpreter/PPCInterpreter.h"

#define DEBUGP std::cout

namespace Xe
{
	namespace Core
	{
		namespace XCPU
		{
			XCPU::XCPU(std::string blPath, Bus* busPointer)
			{  
                cpuContext.bus = busPointer;
                memset(cpuContext.SROM, 0, SROM_SIZE);
                memset(cpuContext.SRAM, 0, SRAM_SIZE);
                memset(cpuContext.mainMem, 0, 25 * 1024 * 1240);
                memset(cpuContext.cpuCores, 0, sizeof(cpuContext.cpuCores));

                for (u8 core = 0; core < 6; core++)
                {
                    cpuContext.cpuCores[core].L1 = new u8[0x17FFFF];
                    memset(cpuContext.cpuCores[core].L1, 0, 0x17FFFF);
                }

                // MSR on reset
                cpuContext.cpuCores[0].MSR.SF = 1;
                cpuContext.cpuCores[0].MSR.HV = 1;

                cpuContext.cpuCores[0].PVR.PVR_Hex = 0x007105000;

                // Set Fuses
                cpuContext.cpuCores[0].CPUFuses[0] = 0xc0ffffffffffffff; // Determines console type
                cpuContext.cpuCores[0].CPUFuses[1] = 0x0f0f0f0f0f0f0ff0; // Retail console      Devkit console 
                                                                         // 0x0f0f0f0f0f0f0ff0  0x0f0f0f0f0f0f0f0f0
                cpuContext.cpuCores[0].CPUFuses[2] = 0x0000000000000000; // Lockdown counter for 2BL/CB
                cpuContext.cpuCores[0].CPUFuses[3] = 0x8CBA33C6B70BF641; // CPU Key - First 32 bits
                cpuContext.cpuCores[0].CPUFuses[4] = 0x8CBA33C6B70BF641; // CPU Key - First 32 bits copy
                cpuContext.cpuCores[0].CPUFuses[5] = 0x2AC5A81E6B41BFE6; // CPU Key - Last 32 bits
                cpuContext.cpuCores[0].CPUFuses[6] = 0x2AC5A81E6B41BFE6; // CPU Key - Last 32 bits copy
                cpuContext.cpuCores[0].CPUFuses[7] = 0xF000000000000000; // From here starts the LDV, this prevents from downgrading console.
                cpuContext.cpuCores[0].CPUFuses[8] = 0x0000000000000000; // each F statnds for an update, 
                cpuContext.cpuCores[0].CPUFuses[9] = 0x0000000000000000;
                cpuContext.cpuCores[0].CPUFuses[10] = 0x0000000000000000;
                cpuContext.cpuCores[0].CPUFuses[11] = 0x0000000000000000;

				if (Load1BL(blPath) != 0)
				{
					return;
				}                 
			}

			bool XCPU::Load1BL(std::string filePath)
			{
				FILE* inputFile;
				fopen_s(&inputFile, filePath.c_str(), "rb");

				if (!inputFile)
				{
					std::cout << "XCPU: Unable to open file: " << filePath << "." << std::endl;
					return 1;
				}

				fseek(inputFile, 0, SEEK_END);
				size_t fileSize = ftell(inputFile);
				fseek(inputFile, 0, SEEK_SET);

				if (fileSize == SROM_SIZE)
				{                  
                    fread(cpuContext.SROM, 1, SROM_SIZE, inputFile);

                    std::cout << "XCPU: 1BL loaded successfully. Entry point at 0x8000020000000100" << std::endl;
				}
                return 0;
			}

            void XCPU::Start(u64 startAddress)
            {
                cpuContext.cpuCores[0].NIA = startAddress;
                cpuContext.cpuCores[0].coreRunning = true;
                cpuContext.executionRunning = true;
                PPCInterpreter::ppcInterpreterExecute(&cpuContext);
            }
		}
	}
}