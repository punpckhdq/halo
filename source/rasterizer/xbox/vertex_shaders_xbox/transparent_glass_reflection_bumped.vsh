xvs.1.1
add r5.xyz, -v0, c[-92]
dph oPos.x, v0, c[-96]
dph oPos.y, v0, c[-95]
dph oPos.z, v0, c[-94]
dph oPos.w, v0, c[-93]
mul oT0.xy, v4, c[-84].xy
mov oT1.x, v3.x
mov oT1.y, v2.x
mov oT1.z, v1.x
mov oT2.x, v3.y
mov oT2.y, v2.y
mov oT2.z, v1.y
mov oT3.x, v3.z
mov oT3.y, v2.z
mov oT3.z, v1.z
dph r8.x, v0, c[-87]
mov oT1.w, r5.x                             
dph r8.y, v0, c[-86]
mov oT2.w, r5.y                             
dph r8.z, v0, c[-88]
mov oT3.w, r5.z                             
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
