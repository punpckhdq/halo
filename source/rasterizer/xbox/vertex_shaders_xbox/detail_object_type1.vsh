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
mov r0.xyz, v8.zzwz
mad r9.xyz, r8, c[-80].xyz, v2
mul r2.xyz, r0.yzxw, c[-91].zxyw
add r10.xyz, r9, -c[-92]
mad r4.xyz, r0.zxyw, -c[-91].yzxw, r2
dp3 r11.w, r10, c[-91]
dp3 r4.w, r4, r4
mad oD0.w, r11.w, r5.y, r5.x
mul r8.xy, r5.zw, r7.zw
rsq r1.w, r4.w                              
mul r10.xy, r8, r3.zw
mul r6.xyz, r4, -r1.w
mad r8.xyz, r6, r10.x, r9
mad r9.xyz, r0, r10.y, r8
mad oT0.xy, r7.xy, r3.zw, r3.xy
dph oPos.x, r9, c[-96]
dph oPos.y, r9, c[-95]
dph oPos.z, r9, c[-94]
dph oPos.w, r9, c[-93]
