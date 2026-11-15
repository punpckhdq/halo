xvs.1.1
mul r2.y, v8.x, c[-81].x
mov a0.x, r2.y
expp r1.y, r2.y                             
mov r3, c[a0.x-36]
mov oD0.xyz, v9                             
mul r4.y, r1.y, c[-81].y
mov a0.x, r4.y
expp r1.y, r4.y                             
mov r5, c[a0.x-75]
mul r6.y, r1.y, c[-81].y
mov a0.x, r6.y
mov r7, c[a0.x-79]
mov r8.xyz, v0
dph r8.z, r8, v3
max r10.w, -c[-91].z, v4.z
mad r9.xyz, r8, c[-80].xyz, v2
mul r11.w, r10.w, r5.w
mul r0.w, r11.w, r3.w
mad r9.z, c[-80].w, r0.w, r9
mul r4.xy, r5.zw, r7.zw
add r2.xyz, r9, -c[-92]
mad oT0.xy, r7.xy, r3.zw, r3.xy
dp3 r4.w, r2, c[-91]
mad oD0.w, r4.w, r5.y, r5.x
mul r5.xy, r4, r3.zw
mad r6.xyz, c[-90], r5.x, r9
mad r8.xyz, c[-89], r5.y, r6
dph oPos.x, r8, c[-96]
dph oPos.y, r8, c[-95]
dph oPos.z, r8, c[-94]
dph oPos.w, r8, c[-93]
