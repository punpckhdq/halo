xvs.1.1
dph oPos.x, v0, c[-96]
dph oPos.y, v0, c[-95]
dph oPos.z, v0, c[-94]
dph oPos.w, v0, c[-93]
mov r2.xyz, v1
mov r3.xyz, v2
mov r4.xyz, v3
add r5.xyz, -v0, c[-92]
mad oD0.xyz, r4, c[-91].w, c[-91].w
dp3 r6.w, r5, r5
mov oT1.xyz, v7                             
dp3 oT0.x, r5, r4
mov r8.xyz, v7
rsq r1.w, r6.w                              
dp3 oD1.w, r8, r8
mul r7.xyz, r5, r1.w
mad oD1.xyz, r7, c[-91].w, c[-91].w
dp3 oT0.y, r5, r3
dp3 oT0.z, r5, r2
dp3 r9.x, r5, r2                            
dp3 oT2.x, v7, r4
dp3 oT2.y, v7, r3
dp3 oT2.z, v7, r2
mul r10.xyz, r9.x, r2
mad oT3.xyz, r10.xyz, c[-92].w, -r5
