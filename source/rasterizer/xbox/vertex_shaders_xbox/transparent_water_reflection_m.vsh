xvs.1.1
mul r2.xy, v5, c[-89].w
mov a0.x, r2.x
mul r3, v6.x, c[a0.x-36]
mul r4, v6.x, c[a0.x-35]
mul r5, v6.x, c[a0.x-34]
mov a0.x, r2.y
add r6.x, v6.w, -v6.x
mad oT0.xy, v4, c[-84].xy, c[-84].zw
mad r7, r6.x, c[a0.x-36], r3
mad r8, r6.x, c[a0.x-35], r4
mad r9, r6.x, c[a0.x-34], r5
dph r10.x, v0, r7
dph r10.y, v0, r8
dph r10.z, v0, r9
dp3 r11.x, v1, r7
dp3 r11.y, v1, r8
dp3 r11.z, v1, r9
dp3 r3.x, v2, r7
dp3 r0.w, r11, r11
dp3 r3.z, v2, r9
dp3 r3.y, v2, r8
rsq r1.w, r0.w                              
dp3 r6.x, v3, r7
mul r2.xyz, r11, r1.w
dp3 r4.w, r3, r3
mov oT1.z, r2.x                             
dp3 r6.z, v3, r9
mov oT3.z, r2                               
dp3 r6.y, v3, r8
rsq r1.w, r4.w                              
add r9.xyz, -r10, c[-92]
mul r5.xyz, r3, r1.w
mov oT2.z, r2.y                             
dp3 r7.w, r6, r6
mov oT1.y, r5.x                             
dph oPos.y, r10, c[-95]
dph oPos.x, r10, c[-96]
rsq r1.w, r7.w                              
dph oPos.z, r10, c[-94]
mul r8.xyz, r6, r1.w
mov oT2.y, r5                               
dph oPos.w, r10, c[-93]
mov oT1.x, r8.x
mov oT2.x, r8.y
mov oT3.x, r8.z
mov oT3.y, r5.z
mov oT1.w, r9.x
mov oT2.w, r9.y
mov oT3.w, r9.z
dph oFog.xw, r10, c[-88]
