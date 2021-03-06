/* Copyright (C) 2012 Ion Torrent Systems, Inc. All Rights Reserved */

#include <assert.h>
#include <iostream>
#include <sstream>
#include "Utils.h"

#include "GpuControlOpts.h"

using namespace std;

void GpuControlOpts::DefaultGpuControl()
{

    gpuWorkLoad = 1.0;
    gpuNumStreams = 2;
    //gpuSharedByNumProcs = 0; //0 default nothing done. if provided GPU memory (minus some padding) will be divided by this number
    gpuDevMemoryPerProc = 0;
    gpuForceMultiFlowFit=false;
    gpuMultiFlowFit = 1;
    gpuThreadsPerBlockMultiFit = 128;
    gpuL1ConfigMultiFit = -1;  // actual default is set hardware specific in MultiFitStream.cu
    gpuThreadsPerBlockPartialD = 128;
    gpuL1ConfigPartialD = -1;  // actual default is set hardware specific in MultiFitStream.cu

    gpuSingleFlowFit = 1;
    gpuThreadsPerBlockSingleFit = -1; // actual default is set hardware specific in SingleFitStream.cu
    gpuL1ConfigSingleFit = -1; // actual default is set hardware specific in SingleFitStream.cu

    // 0: GaussNewton, 1: LevMar 2:Hybrid (gpuHybridIterations Gauss Newton, then rest LevMar)
    // 3: Relaxing Kmult (two pass Gauss Newton)
    gpuSingleFlowFitType = 3;
    gpuHybridIterations = 3;

    doGpuOnlyFitting = 1;

    gpuAmpGuess = 1;

    gpuUseAllDevices=false;
    gpuVerbose = false;

    gpuFlowByFlowExecution = false;
    postFitHandshakeWorker = false;
    switchToFlowByFlowAt = 20;
    gpuNumHistoryFlows = 10;
}

void GpuControlOpts::PrintHelp()
{
	printf ("     GpuControlOpts\n");
  printf ("     --gpuworkload           FLOAT             gpu work load [1.0]\n");
  printf ("     --gpuWorkLoad           FLOAT             same as --gpuworkload [1.0]\n");
	printf ("     --gpu-verbose           BOOL              gpu verbose [false]\n");
	printf ("     --gpu-use-all-devices   BOOL              forces use of all available GPUs passing minimum requirements. may cause non-deterministic results if GPUs have varying compute version [false]\n");
	printf ("     --gpu-fitting-only      BOOL              do gpu only fitting [true]\n");
	printf ("     --gpu-flow-by-flow      BOOL              use new flow by flow GPU pipeline [false]\n");
	printf ("     --post-fit-handshake-worker      BOOL     post fit steps in separate thread [false]\n");
	printf ("     --gpu-num-history-flows    INT            number of history flows to perform regional fitting\n");
  printf ("     --gpu-tmidnuc-shift-per-flow     BOOL     do t_mid_nuc shift per flow [true]\n");
	printf ("     --gpu-device-ids        INT               gpu device ids []\n");
	printf ("     --gpu-num-streams       INT               gpu num streams [2]\n");
	//printf ("     --gpu-shared-by-num-procs        INT      number of processes sharing the gpu, if provided gpu memory per process will be fixed as: gpu memory/num procs\n");
	printf ("     --gpu-memory-per-proc   INT               request a minimum of <int>MB of device memory per process, if provided overwrites dynamic memory allocation\n");
	printf ("     --gpu-amp-guess         INT               gpu amp guess [1]\n");
	printf ("     --gpu-hybrid-fit-iter   INT               gpu hybrid fit iteration [3]\n");
	printf ("     --gpu-single-flow-fit   INT               gpu single flow fit [1]\n");
	printf ("     --gpu-multi-flow-fit    INT               gpu multi flow fit [1]\n");
	printf ("     --gpu-force-multi-flow-fit          BOOL  force multi flow fit execution despite any other setting (for Vadim's use only)\n");
	printf ("     --gpu-single-flow-fit-blocksize     INT   gpu threads per block single fit []\n");
	printf ("     --gpu-multi-flow-fit-blocksize      INT   gpu threads per block multi fit [128]\n");
	printf ("     --gpu-single-flow-fit-l1config      INT   gpu L1 config single fit []\n");
	printf ("     --gpu-multi-flow-fit-l1config       INT   gpu L1 config multi fit []\n");
	printf ("     --gpu-single-flow-fit-type          INT   gpu single flow fit type [3]\n");
	printf ("     --gpu-partial-deriv-blocksize       INT   gpu threads per block partialD [128]\n");
	printf ("     --gpu-partial-deriv-l1config        INT   gpu L1 config partialD []\n");
  printf ("     --gpu-switch-to-flow-by-flow-at     INT   if using flow by flow pipeline switch at flow [20]\n");
    printf ("\n");
}

void GpuControlOpts::SetOpts(OptArgs &opts, Json::Value& json_params)
{
  gpuWorkLoad = RetrieveParameterFloat(opts, json_params, '-', "gpuworkload", 1.0);
  if ( ( gpuWorkLoad > 1 ) || ( gpuWorkLoad < 0 ) )
  {
    fprintf ( stderr, "Option Error: gpuworkload must specify a value between 0 and 1 (%f invalid).\n", gpuWorkLoad );
    exit ( EXIT_FAILURE );
  }
  gpuNumStreams = RetrieveParameterInt(opts, json_params, '-', "gpu-num-streams", 2);
  if ( ( gpuNumStreams < 1 ) && ( gpuNumStreams > 16 ) )
  {
    fprintf ( stderr, "Option Error: gpu-num-streams must specify a value between 1 and 16 (%d invalid).\n", gpuNumStreams );
    exit ( EXIT_FAILURE );
  }
  gpuAmpGuess = RetrieveParameterInt(opts, json_params, '-', "gpu-amp-guess", 1);
  if ( gpuAmpGuess != 0 && gpuAmpGuess != 1 )
  {
    fprintf ( stderr, "Option Error: gpu-amp-guess must be either 0 or 1 (%d invalid).\n",gpuAmpGuess );
    exit ( EXIT_FAILURE );
  }
//  gpuSharedByNumProcs = RetrieveParameterInt(opts, json_params, '-', "gpu-shared-by-num-procs", 0);
//  if ( gpuSharedByNumProcs < 0  )
//  {
//    fprintf ( stderr, "Option Error: gpu-shared-by-num-procs must be >= 0, where 0 will do dynamic memory allocation (%d invalid).\n",gpuSharedByNumProcs );
//    exit ( EXIT_FAILURE );
//  }
  gpuDevMemoryPerProc = RetrieveParameterInt(opts, json_params, '-', "gpu-memory-per-proc", 0);
  if ( gpuDevMemoryPerProc < 0  )
  {
    fprintf ( stderr, "Option Error: gpu-memory-per-proc must be >= 0, where 0 will do dynamic memory allocation (%d invalid).\n",gpuDevMemoryPerProc );
    exit ( EXIT_FAILURE );
  }
  gpuSingleFlowFit = RetrieveParameterInt(opts, json_params, '-', "gpu-single-flow-fit", 1);
  if ( gpuSingleFlowFit != 0 && gpuSingleFlowFit != 1 )
  {
    fprintf ( stderr, "Option Error: gpu-single-flow-fit must be either 0 or 1 (%d invalid).\n", gpuSingleFlowFit );
    exit ( EXIT_FAILURE );
  }
  gpuThreadsPerBlockSingleFit = RetrieveParameterInt(opts, json_params, '-', "gpu-single-flow-fit-blocksize", -1);
  if(gpuThreadsPerBlockSingleFit >= 0)
  {
    if ( gpuThreadsPerBlockSingleFit <= 0 )
    {
      fprintf ( stderr, "Option Error: gpu-single-flow-fit-blocksize must be > 0 (%d invalid).\n", gpuThreadsPerBlockSingleFit );
      exit ( EXIT_FAILURE );
    }
  }
  gpuL1ConfigSingleFit = RetrieveParameterInt(opts, json_params, '-', "gpu-single-flow-fit-l1config", -1);
  gpuMultiFlowFit = RetrieveParameterInt(opts, json_params, '-', "gpu-multi-flow-fit", 1);
  gpuForceMultiFlowFit = RetrieveParameterBool(opts, json_params, '-', "gpu-force-multi-flow-fit", false);
  if ( gpuMultiFlowFit != 0 && gpuMultiFlowFit != 1 )
  {
    fprintf ( stderr, "Option Error: gpu-multi-flow-fit must be either 0 or 1 (%d invalid).\n", gpuMultiFlowFit );
    exit ( EXIT_FAILURE );
  }
  gpuThreadsPerBlockMultiFit = RetrieveParameterInt(opts, json_params, '-', "gpu-multi-flow-fit-blocksize", 128);
  if ( gpuThreadsPerBlockMultiFit <= 0 )
  {
    fprintf ( stderr, "Option Error: gpu-multi-flow-fit-blocksize must be > 0 (%d invalid).\n", gpuThreadsPerBlockMultiFit );
    exit ( EXIT_FAILURE );
  }
  gpuL1ConfigMultiFit = RetrieveParameterInt(opts, json_params, '-', "gpu-multi-flow-fit-l1config", -1);
  gpuSingleFlowFitType = RetrieveParameterInt(opts, json_params, '-', "gpu-single-flow-fit-type", 3);
  gpuHybridIterations = RetrieveParameterInt(opts, json_params, '-', "gpu-hybrid-fit-iter", 3);
  gpuThreadsPerBlockPartialD = RetrieveParameterInt(opts, json_params, '-', "gpu-partial-deriv-blocksize", 128);
  if ( gpuThreadsPerBlockPartialD <= 0 )
  {
    fprintf ( stderr, "Option Error: gpu-partial-deriv-blocksize must be > 0 (%d invalid).\n", gpuThreadsPerBlockPartialD );
    exit ( EXIT_FAILURE );
  }
  gpuL1ConfigPartialD = RetrieveParameterInt(opts, json_params, '-', "gpu-partial-deriv-l1config", -1);
  gpuUseAllDevices = RetrieveParameterBool(opts, json_params, '-', "gpu-use-all-devices", false);
  gpuVerbose = RetrieveParameterBool(opts, json_params, '-', "gpu-verbose", false);
  vector<int> deviceIds;
  RetrieveParameterVectorInt(opts, json_params, '-', "gpu-device-ids", "", deviceIds);
  for (size_t i = 0; i < deviceIds.size(); ++i)
  {
    gpuDeviceIds.push_back(deviceIds[i]);
  }
  if (deviceIds.size() > 0)
  {
    std::sort(gpuDeviceIds.begin(), gpuDeviceIds.end());
  }

  //jz the following comes from CommandLineOpts::GetOpts
  doGpuOnlyFitting = RetrieveParameterBool(opts, json_params, '-', "gpu-fitting-only", true);
  gpuFlowByFlowExecution = RetrieveParameterBool(opts, json_params, '-', "gpu-flow-by-flow", false);

  if (gpuFlowByFlowExecution) {
    postFitHandshakeWorker = RetrieveParameterBool(opts, json_params, '-', "post-fit-handshake-worker", true);
    switchToFlowByFlowAt = RetrieveParameterInt(opts, json_params, '-', "gpu-switch-to-flow-by-flow-at", 20);
    gpuNumHistoryFlows = RetrieveParameterInt(opts, json_params, '-', "gpu-num-history-flows", 10);
  }
  else {
    postFitHandshakeWorker = false;
  }
}
