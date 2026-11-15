xvs.1.1
dph oPos.x, v0, c[-68]
dph oPos.y, v0, c[-67]
dph oPos.z, v0, c[-66]
dph oPos.w, v0, c[-65]
mov oD0, v9
mul r10.xy, c[-62].x, v0.xy
mad r10.xy, v4, c[-62].y, r10.xy
mul r10.xy, r10.xy, c[-59].xy
add r10.xy, r10.xy, c[-61].zw
mul oT0.xy, r10.xy, c[-64].xy
mul r10.xy, c[-62].z, v0.xy
mad r10.xy, v4, c[-62].w, r10.xy
mul r10.xy, r10.xy, c[-59].zw
add r10.xy, r10.xy, c[-60].xy
mul oT1.xy, r10.xy, c[-63].xy
mul r10.xy, c[-61].x, v0.xy
mad r10.xy, v4, c[-61].y, r10.xy
mul r10.xy, r10.xy, c[-58].xy
add r10.xy, r10.xy, c[-60].zw
mul oT2.xy, r10.xy, c[-63].zw
