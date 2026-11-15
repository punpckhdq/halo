xvs.1.1
dph oPos.x, v0, c[-96]
dph oPos.y, v0, c[-95]
dph oPos.z, v0, c[-94]
dph oPos.w, v0, c[-93]
mov r2.xyz, v1
mov r3.xyz, v2
mov r4.xyz, v3
add r5.xyz, -v0, c[-92]
mad oD1.xyz, v7, c[-91].w, c[-91].w
dp3 r6.w, r5, r5
dp3 r10.x, r2, r4
dp3 r9.x, r2, r3
rsq r1.w, r6.w                              
mad oD1.w, r10.x, c[-91].w, c[-91].w
mul r7.xyz, r5, r1.w
dp3 r8.x, r7, r4
dp3 r8.y, r7, r3
dp3 r8.z, r7, r2
mad oD0.w, r9.x, c[-91].w, c[-91].w
mad oD0.xyz, r8, c[-91].w, c[-91].w
dp3 r11.x, r3, r4
dp3 oT0.x, r2, c[-90]
mad oFog.xw, r11.x, c[-91].w, c[-91].w
dp3 oT0.y, r2, c[-89]
dp3 oT0.z, r2, c[-91]
dp3 oT1.x, r3, c[-90]
dp3 oT1.y, r3, c[-89]
dp3 oT1.z, r3, c[-91]
dp3 oT2.x, r4, c[-90]
dp3 oT2.y, r4, c[-89]
dp3 oT2.z, r4, c[-91]
dp3 oT3.x, v7, c[-90]
dp3 oT3.y, v7, c[-89]
dp3 oT3.z, v7, c[-91]
