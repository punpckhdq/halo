xvs.1.1
add r5.xyz, -v0, c[-92]
dph oPos.x, v0, c[-96]
dph oPos.y, v0, c[-95]
dph oPos.z, v0, c[-94]
dph oPos.w, v0, c[-93]
mad oT0.xy, v4, c[-84].xy, c[-84].zw
mov oT1.x, v3.x
mov oT1.y, v2.x
mov oT1.z, v1.x
mov oT2.x, v3.y
mov oT2.y, v2.y
mov oT2.z, v1.y
mov oT3.x, v3.z
mov oT3.y, v2.z
mov oT3.z, v1.z
mov oT1.w, r5.x
mov oT2.w, r5.y
mov oT3.w, r5.z
dph oFog.xw, v0, c[-88]
