//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

struct PSInput
{
    float4 position : SV_POSITION;
    float4 normal :NORMAL;
    float4 uv : UV;
};

PSInput VSMain(float4 position : POSITION, float4 normal : NORMAL, float4 uv : UV )
{
    PSInput result;

    result.position = position;
    result.normal = normal;
    result.uv = uv;

    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    return float4(1.0f, 0.0f, 0.0f, 1.0f);
}
