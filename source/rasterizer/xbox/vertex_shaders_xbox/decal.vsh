xvs.1.1
dph oPos.x, v0, c[-96]
dph oPos.y, v0, c[-95]
dph oPos.z, v0, c[-94]
dph oPos.w, v0, c[-93]
mov oT0.xy, v4
mov oD0, v9
dph r8.z, v0, c[-88]
min r8.z, r8.z, v4.w
mul r8.z, r8.z, c[-85].x
slt oD1.w, r8.z, v4.w
