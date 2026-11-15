xvs.1.1
mul r2.xy, v5, c[-89].w
mov a0.x, r2
mov oT1.y, v4.z                             
mul r3, v6.x, c[a0.x-36]
mul r4, v6.x, c[a0.x-35]
mul r5, v6.x, c[a0.x-34]
mov a0.x, r2.y
mov oT2.x, v4.z                             
add r6.x, v6.w, -v6.x
mov oD0.xyz, c[-82]
mad r7, r6.x, c[a0.x-36], r3
mad r8, r6.x, c[a0.x-35], r4
mad r9, r6.x, c[a0.x-34], r5
dph r10.x, v0, r7
dph r10.y, v0, r8
dph r10.z, v0, r9
dp3 r11.x, v1, r7
dp3 r11.y, v1, r8
dp3 r11.z, v1, r9
add r3.xyz, -r10, c[-92]
dp3 r0.w, r11, r11
dph oPos.x, r10, c[-96]
dph r9.x, r10, c[-96]                       
dp3 r4.w, r3, c[-91]
rsq r1.w, r0.w                              
dph oPos.y, r10, c[-95]
dph r9.y, r10, c[-95]                       
mul r2.xyz, r11, r1.w
mul r5.w, r4.w, r4.w
dph oPos.z, r10, c[-94]
rcp r1.w, r5.w                              
dph oPos.w, r10, c[-93]
dph r9.w, r10, c[-93]                       
mul r6.w, r1.w, c[-84].y
min r7.w, r6.w, v4.w
max oD0.w, r7.w, v4.z
max r8.w, r7.w, v4.z                        
dp3 oT0.x, r2, c[-90]
dp3 oT0.y, r2, c[-89]
dp3 oT0.z, r2, c[-91]
mul oT1.x, r8.w, c[-84].x
mul oT2.y, r8.w, c[-84].x
add r10.xyw, r9.xyw, r9.w
rcp r1.w, r10.w
mul r11.xy, r10, r1.w
mul oT1.z, c[-84].z, r11.x
mul oT2.z, c[-84].w, r11.y
