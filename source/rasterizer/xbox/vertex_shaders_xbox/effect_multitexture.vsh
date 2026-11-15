xvs.1.1
dph r0.x, v0, c88
dph r0.y, v0, c89
dph r0.z, v0, c90
dp4 oT1.x, v4, c[-79]
dph r8.x, r0, c[-87]
mov oT0.xy, v4                              
dph oPos.x, r0, c[-96]
dph oPos.y, r0, c[-95]
dph oPos.z, r0, c[-94]
dph oPos.w, r0, c[-93]
dp4 oT1.y, v4, c[-78]
dph r8.y, r0, c[-86]
mov oD1, v9                                 
dph r8.z, r0, c[-88]
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
