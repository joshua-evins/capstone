__kernel void NBody(float dtl, float eps,
	__global float4* pos_old,
	__global float4* pos_new,
	__global float4* vel,
	__local float4* pblock)
{
	int gi = get_global_id(0);
	int li = get_local_id(0);
	
	int gnumThreads = get_global_size(0);
	int lnumThreads = get_local_size(0);
	int numBlocks = gnumThreads/lnumThreads;
	
	float4 p = pos_old[gi];
	float4 v = vel[gi];
	
	float4 a = (float4)(0.0f, 0.0f, 0.0f, 0.0f);
	
	for(int ib=0; ib<numBlocks; ib++)
	{
		pblock[li] = pos_old[ib*lnumThreads + li];
		
		barrier(CLK_LOCAL_MEM_FENCE);
		
		for(int ic=0; ic<lnumThreads; ic++)
		{
			float4 p2 = pblock[ic];
			float4 d = p2-p;
			float invr = rsqrt(d.x*d.x + d.y*d.y + d.z*d.z + eps);
			float f = p2.w * invr * invr * invr;
			a += f*d;
		}
		
		barrier(CLK_LOCAL_MEM_FENCE);
	}
	p += dtl*v + 0.5f * dtl * dtl * a;
	v += dtl*a;
	
	pos_new[gi] = p;
	vel[gi] = v*0.99f;
}