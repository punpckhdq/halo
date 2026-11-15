xvs.1.1
mul r0.xy, v5, c[-89].w
mov a0.x, r0.x
mul r4, v6.x, c[a0.x-36]
mul r5, v6.x, c[a0.x-35]
mul r6, v6.x, c[a0.x-34]
mov a0.x, r0.y
add r9.x, v6.w, -v6.x
dph oT0.x, v4, c[-83]
mad r4, r9.x, c[a0.x-36], r4
mad r5, r9.x, c[a0.x-35], r5
mad r6, r9.x, c[a0.x-34], r6
dph r0.x, v0, r4
dph r0.y, v0, r5
dph r0.z, v0, r6
dph oT0.y, v4, c[-82]
dph oPos.x, r0, c[-96]
dph oPos.y, r0, c[-95]
dph oPos.z, r0, c[-94]
dph oPos.w, r0, c[-93]
