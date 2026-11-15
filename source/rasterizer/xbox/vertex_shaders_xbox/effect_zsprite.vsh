xvs.1.1
dph r0.x, v0, c88
dph r0.y, v0, c89
dph r0.z, v0, c90
dph r8.y, r0, c[-86]
mov oT1.xy, v11                             
dph r10.z, r0, c[-62]
mov oT0.xy, v4                              
dph oPos.x, r0, c[-96]
dph oPos.y, r0, c[-95]
dph oPos.z, r0, c[-94]
dph oPos.w, r0, c[-93]
mov r10.y, r10.z
mov oT2.xw, v4.z                            
mov oT3.z, r10
add r10.y, -c[-63].w, r10                   
mad oT2.z, c[-63].x, r10.z, c[-63].y
sge r11.z, c[-63], r10.y
mov oT3.xw, v4.z                            
add r11.w, v4.w, -r11.z
mul r10.w, r11.z, r10.y
mov oD1, v9                                 
mad r10.w, r11.w, c[-63].z, r10.w
dph r8.z, r0, c[-88]
dph r8.x, r0, c[-87]
mov oT3.y, -r10.w                           
mul oT2.y, -c[-63].x, r10.w
add r8.xy, v4.w, -r8
max r8.xyz, r8, v4.z
mul r8.xy, r8, r8
min r8.xyz, r8, v4.w
add r8.x, r8.x, r8.y
mul r8.z, r8.z, c[-85].x
min r8.x, r8, v4.w
add r8.xy, v4.w, -r8
mul r8.xy, r8, r8
add r8.y, r8.y, -r8.x
mad r8.w, c[-85].y, r8.y, r8.x
mul r8.w, r8.w, c[-85].z
add r8.zw, -r8, v4.w
mul oD0.w, r8.z, r8.w
