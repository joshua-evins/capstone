
// sphFluidDemo.cl  A. Heirich   11/8/2010
//
// Equations referenced here are from:
// "Particle-based fluid simulation for interactive applications", Muller, Charypar & Gross,
// Eurographics/SIGGRAPH Symposium on Computer Animation (2003).


#define PARTICLE_COUNT ( 8 * 1024 )
#define NEIGHBOR_COUNT 32

#define NO_PARTICLE_ID -1
#define NO_CELL_ID -1
#define NO_DISTANCE -1.0f

#define POSITION_CELL_ID( i ) i.w

#define PI_CELL_ID( name ) name.x
#define PI_SERIAL_ID( name ) name.y

#define NEIGHBOR_MAP_ID( nm ) nm.x
#define NEIGHBOR_MAP_DISTANCE( nm ) nm.y

#define RHO( i ) i.x
#define RHO_INV( i ) i.y
#define P( i ) i.z

#define DIVIDE( a, b ) native_divide( a, b )
#define SQRT( x ) native_sqrt( x + 0.1 )
#define DOT( a, b ) dot( a, b )

#define SELECT( A, B, C ) C ? B : A

__kernel void clearBuffers(
						   __global float2 * neighborMap,
						   __global uint* totalOnes
						   )
{
	int id = get_global_id( 0 )*8;
	
	float16 fdata =
	(float16)(
	-1, -1, -1, -1,
	-1, -1, -1, -1,
	-1, -1, -1, -1,
	-1, -1, -1, -1
	);
	((__global float16*)(neighborMap+id))[0] = fdata;
}

// Gradient of equation 21.  Vector result.

float4 gradWspiky(
				  float r,
				  float h,
				  float gradWspikyCoefficient,
				  float4 position_i,
				  float4 position_j,
				  float simulationScale
				  )
{
	float4 rVec = position_i - position_j;
	float4 scaledVec = rVec * simulationScale;
	scaledVec /= r;
	rVec.w = 0.0f;
	float x = h - r;
	float4 result = x * x * scaledVec * gradWspikyCoefficient;
	return result;
}



float4 contributeGradP(
					   int id,
					   int neighborParticleId,						
					   float p_i,
					   float p_j,
					   float rho_j_inv,
					   float4 position_i,
					   __global float * pressure,
					   __global float * rho,
					   __global float4 * sortedPosition,
					   float r,
					   float mass,
					   float h,
					   float gradWspikyCoefficient,
					   float simulationScale
					   )
{
	// Following Muller Charypis and Gross ( 2003 )
	// -grad p_i = - sum_j m_j ( p_i + p_j ) / ( 2 rho_j ) grad Wspiky
	// Equation 10.
	float4 neighborPosition;
	neighborPosition = sortedPosition[ neighborParticleId ];
	float4 smoothingKernel = gradWspiky( r, h, gradWspikyCoefficient, position_i, neighborPosition, simulationScale );
	float4 result = mass * ( p_i + p_j ) * 0.5f * rho_j_inv * smoothingKernel;
	return result;
}


// Laplacian of equation 22.  Scalar result.

float del2Wviscosity(
					 float r,
					 float h,
					 float del2WviscosityCoefficient
					 )
{
	// equation 22
	float result = ( h - r ) * del2WviscosityCoefficient;
	return result;
}



float4 contributeDel2V(
					   int id,
					   float4 v_i,
					   int neighborParticleId,
					   __global float4 * sortedVelocity,
					   float rho_j_inv,
					   float r,
					   float mass,
					   float h,
					   float del2WviscosityCoefficient
					   )
{
	// mu del^2 v = mu sum_j m_j ( v_j - v_i ) / rho_j del^2 Wviscosity
	// Equation 14.
	float4 v_j = sortedVelocity[ neighborParticleId ];
	float4 d = v_j - v_i;
	float4 result = mass * d * rho_j_inv * del2Wviscosity( r, h, del2WviscosityCoefficient );
	return result;
}




__kernel void computeAcceleration(
	__global float2 * neighborMap,
	__global float * pressure,
	__global float * rho,
	__global float * rhoInv,
	__global float4 * sortedPosition,
	__global float4 * sortedVelocity,
	float CFLLimit,
	float del2WviscosityCoefficient,
	float gradWspikyCoefficient,
	float h,
	float mass,
	float mu,
	float simulationScale,
	__global float4 * acceleration
	)
{
	int id = get_global_id( 0 );
	int idk = id * NEIGHBOR_COUNT;
	float hScaled = h * simulationScale;

	
	
	float4 position_i = sortedPosition[ id ];
	float4 velocity_i = sortedVelocity[ id ];
	
	float16 nmIn[4];
	nmIn[0] = ((__global float16*)(neighborMap+idk))[0];
	nmIn[1] = ((__global float16*)(neighborMap+idk))[1];
	nmIn[2] = ((__global float16*)(neighborMap+idk))[2];
	nmIn[3] = ((__global float16*)(neighborMap+idk))[3];
	float2 nms[32];
	nms[0] = nmIn[0].s01;
	nms[1] = nmIn[0].s23;
	nms[2] = nmIn[0].s45;
	nms[3] = nmIn[0].s67;
	nms[4] = nmIn[0].s89;
	nms[5] = nmIn[0].sab;
	nms[6] = nmIn[0].scd;
	nms[7] = nmIn[0].sef;
	
	nms[8] = nmIn[1].s01;
	nms[9] = nmIn[1].s23;
	nms[10] = nmIn[1].s45;
	nms[11] = nmIn[1].s67;
	nms[12] = nmIn[1].s89;
	nms[13] = nmIn[1].sab;
	nms[14] = nmIn[1].scd;
	nms[15] = nmIn[1].sef;
	
	nms[16] = nmIn[2].s01;
	nms[17] = nmIn[2].s23;
	nms[18] = nmIn[2].s45;
	nms[19] = nmIn[2].s67;
	nms[20] = nmIn[2].s89;
	nms[21] = nmIn[2].sab;
	nms[22] = nmIn[2].scd;
	nms[23] = nmIn[2].sef;
	
	nms[24] = nmIn[3].s01;
	nms[25] = nmIn[3].s23;
	nms[26] = nmIn[3].s45;
	nms[27] = nmIn[3].s67;
	nms[28] = nmIn[3].s89;
	nms[29] = nmIn[3].sab;
	nms[30] = nmIn[3].scd;
	nms[31] = nmIn[3].sef;
	
	float p_i = pressure[ id ];
	float rho_i_inv = rhoInv[ id ];
	float4 result = (float4)( 0.0f, 0.0f, 0.0f, 0.0f );
	
	float4 gradP = (float4)( 0.0f, 0.0f, 0.0f, 0.0f );
	float4 del2V = (float4)( 0.0f, 0.0f, 0.0f, 0.0f );
	float2 nm;

	NEIGHBOR_MAP_ID( nm ) = id;
	NEIGHBOR_MAP_DISTANCE( nm ) = 0.0f;

	int j = 0;
	bool loop;
	do{
		nm = nms[ j ];
		int neighborParticleId = NEIGHBOR_MAP_ID( nm );
		bool isNeighbor = ( neighborParticleId != NO_PARTICLE_ID );
		if( isNeighbor ){
			float p_j = pressure[ neighborParticleId ];
			float rho_j_inv = rhoInv[ neighborParticleId ];
			float r = NEIGHBOR_MAP_DISTANCE( nm );
			float4 dgradP = contributeGradP( id, neighborParticleId, p_i, p_j, rho_j_inv,
				position_i, pressure, rho, sortedPosition, r, mass, hScaled,
				gradWspikyCoefficient, simulationScale );
			float4 ddel2V = contributeDel2V( id, velocity_i, neighborParticleId,
				sortedVelocity, rho_j_inv, r, mass, hScaled, del2WviscosityCoefficient );
			gradP += dgradP;
			del2V += ddel2V;
		}
		loop = ( ++j < NEIGHBOR_COUNT );
	}while( loop );

	result = rho_i_inv * ( mu * del2V - gradP );

	// Check CFL condition
	float magnitude = result.x * result.x + result.y * result.y + result.z * result.z;
	bool tooBig = ( magnitude > CFLLimit * CFLLimit );
	float sqrtMagnitude = SQRT( magnitude );
	float scale = CFLLimit / sqrtMagnitude;
	result = SELECT(result, result*scale, tooBig);
	result.w = 0.0f;
	acceleration[ id ] = result;
}





// Mueller et al equation 3.  Scalar result.

float Wpoly6(
			 float r,
			 float hSquared,
			 float Wpoly6Coefficient
			 )
{
	float x = hSquared - r * r;
	float result = x * x * x * Wpoly6Coefficient;
	return result;
}



float densityContribution(
						  int neighborParticleId,
						  float r,
						  float mass,
						  float hSquared,
						  float Wpoly6Coefficient
						  )
{
	float smoothingKernel = Wpoly6( r, hSquared, Wpoly6Coefficient );
	float result = SELECT( smoothingKernel, 0.0f, ( neighborParticleId == NO_PARTICLE_ID )  );
	return result;
}



__kernel void computeDensityPressure(
	__global float2 * neighborMap,
	float Wpoly6Coefficient,
	float h,
	float mass,
	float rho0,
	float simulationScale,
	float stiffness,
	__global float * pressure,
	__global float * rho,
	__global float * rhoInv
	)
{
	int id = get_global_id( 0 );
	int idx = id * NEIGHBOR_COUNT;
	float density = 0.0f;
	float hScaled = h * simulationScale;
	float hSquared = hScaled * hScaled;

	__global float16* neighborBase = (__global float16*)(neighborMap+idx);
	
	float16 nquads[4];
	nquads[0] = neighborBase[0];
	nquads[1] = neighborBase[1];
	nquads[2] = neighborBase[2];
	nquads[3] = neighborBase[3];
	
	float16 nquad = nquads[0];
	density = density + densityContribution( nquad.s0, nquad.s1, mass, hSquared, Wpoly6Coefficient );
	density = density + densityContribution( nquad.s2, nquad.s3, mass, hSquared, Wpoly6Coefficient );
	density = density + densityContribution( nquad.s4, nquad.s5, mass, hSquared, Wpoly6Coefficient );
	density = density + densityContribution( nquad.s6, nquad.s7, mass, hSquared, Wpoly6Coefficient );
	density = density + densityContribution( nquad.s8, nquad.s9, mass, hSquared, Wpoly6Coefficient );
	density = density + densityContribution( nquad.sa, nquad.sb, mass, hSquared, Wpoly6Coefficient );
	density = density + densityContribution( nquad.sc, nquad.sd, mass, hSquared, Wpoly6Coefficient );
	density = density + densityContribution( nquad.se, nquad.sf, mass, hSquared, Wpoly6Coefficient );
	
	nquad = nquads[1];
	density = density + densityContribution( nquad.s0, nquad.s1, mass, hSquared, Wpoly6Coefficient );
	density = density + densityContribution( nquad.s2, nquad.s3, mass, hSquared, Wpoly6Coefficient );
	density = density + densityContribution( nquad.s4, nquad.s5, mass, hSquared, Wpoly6Coefficient );
	density = density + densityContribution( nquad.s6, nquad.s7, mass, hSquared, Wpoly6Coefficient );
	density = density + densityContribution( nquad.s8, nquad.s9, mass, hSquared, Wpoly6Coefficient );
	density = density + densityContribution( nquad.sa, nquad.sb, mass, hSquared, Wpoly6Coefficient );
	density = density + densityContribution( nquad.sc, nquad.sd, mass, hSquared, Wpoly6Coefficient );
	density = density + densityContribution( nquad.se, nquad.sf, mass, hSquared, Wpoly6Coefficient );
	
	nquad = nquads[2];
	density = density + densityContribution( nquad.s0, nquad.s1, mass, hSquared, Wpoly6Coefficient );
	density = density + densityContribution( nquad.s2, nquad.s3, mass, hSquared, Wpoly6Coefficient );
	density = density + densityContribution( nquad.s4, nquad.s5, mass, hSquared, Wpoly6Coefficient );
	density = density + densityContribution( nquad.s6, nquad.s7, mass, hSquared, Wpoly6Coefficient );
	density = density + densityContribution( nquad.s8, nquad.s9, mass, hSquared, Wpoly6Coefficient );
	density = density + densityContribution( nquad.sa, nquad.sb, mass, hSquared, Wpoly6Coefficient );
	density = density + densityContribution( nquad.sc, nquad.sd, mass, hSquared, Wpoly6Coefficient );
	density = density + densityContribution( nquad.se, nquad.sf, mass, hSquared, Wpoly6Coefficient );
	
	nquad = nquads[3];
	density = density + densityContribution( nquad.s0, nquad.s1, mass, hSquared, Wpoly6Coefficient );
	density = density + densityContribution( nquad.s2, nquad.s3, mass, hSquared, Wpoly6Coefficient );
	density = density + densityContribution( nquad.s4, nquad.s5, mass, hSquared, Wpoly6Coefficient );
	density = density + densityContribution( nquad.s6, nquad.s7, mass, hSquared, Wpoly6Coefficient );
	density = density + densityContribution( nquad.s8, nquad.s9, mass, hSquared, Wpoly6Coefficient );
	density = density + densityContribution( nquad.sa, nquad.sb, mass, hSquared, Wpoly6Coefficient );
	density = density + densityContribution( nquad.sc, nquad.sd, mass, hSquared, Wpoly6Coefficient );
	density = density + densityContribution( nquad.se, nquad.sf, mass, hSquared, Wpoly6Coefficient );

	density *= mass; // since all particles are same fluid type, factor this out to here
	rho[ id ] = density; // my density
	rhoInv[ id ] = SELECT( 1.0f, DIVIDE( 1.0f, density ), ( density > 0.0f ) );
	float drho = density - rho0; // rho0 is resting density
	float k = stiffness;
	float p = k * drho; // equation 12
	pressure[ id ] = p; // my pressure

}




int searchCell( 
			   int cellId,
			   int deltaX,
			   int deltaY,
			   int deltaZ,
			   int gridCellsX, 
			   int gridCellsY, 
			   int gridCellsZ,
			   int gridCellCount
			   )
{
	int dx = deltaX;
	int dy = deltaY * gridCellsX;
	int dz = deltaZ * gridCellsX * gridCellsY;
	int newCellId = cellId + dx + dy + dz;
	newCellId = SELECT( newCellId, newCellId + gridCellCount, ( newCellId < 0 ) );
	newCellId = SELECT( newCellId, newCellId - gridCellCount, ( newCellId >= gridCellCount ) );
	return newCellId;
}


#define FOUND_NO_NEIGHBOR 0
#define FOUND_ONE_NEIGHBOR 1


int considerParticle(
					 int cellId,
					 int neighborParticleId,
					 float4 position_,
					 int myParticleId,
					 __global float4 * sortedPosition,
					 __global uint * gridCellIndex,
					 __global float2 * neighborMap, 
					 int myOffset,
					 float h,
					 float simulationScale
					 )
{
	float4 neighborPosition;
	neighborPosition = sortedPosition[ neighborParticleId ];
	float4 d = position_ - neighborPosition;
	d.w = 0.0f;
	float distanceSquared = DOT( d, d );
	float distance = SQRT( distanceSquared );
	bool tooFarAway = ( distance > h );
	bool neighborIsMe = ( neighborParticleId == myParticleId );
	if( tooFarAway || neighborIsMe ){
		return FOUND_NO_NEIGHBOR;
	}

	float scaledDistance = distance * simulationScale;
	float2 myMapEntry;
	NEIGHBOR_MAP_ID( myMapEntry ) = neighborParticleId;
	NEIGHBOR_MAP_DISTANCE( myMapEntry ) = scaledDistance;
	int myIdx = myParticleId * NEIGHBOR_COUNT + myOffset;
	neighborMap[ myIdx ] = myMapEntry;
	return FOUND_ONE_NEIGHBOR;
}




uint myRandom( 
			  uint prior,
			  int maxParticles
			  )
{
	//unsigned long int m = PARTICLE_COUNT;//generator period, assume power of 2
	unsigned long int a = 1664525;
	unsigned long int c = 1013904223;
	uint result = (uint)(( a * prior + c ) % maxParticles );
	return result;
}




int searchForNeighbors( 
					   int searchCell_, 
					   __global uint * gridCellIndex, 
					   float4 position_, 
					   int myParticleId, 
					   __global float4 * sortedPosition,
					   __global float2 * neighborMap,
					   int spaceLeft,
					   float h,
					   float simulationScale
					   )
{
	//float h = h2*1.0f;
	int baseParticleId = gridCellIndex[ searchCell_ ];
	int nextParticleId = gridCellIndex[ searchCell_ + 1 ];
	int particleCountThisCell = nextParticleId - baseParticleId;
	int particleOffset = SELECT( 0, myRandom( baseParticleId + myParticleId, PARTICLE_COUNT ) % particleCountThisCell,
		( particleCountThisCell > 0 ) );

	int potentialNeighbors = particleCountThisCell;
	bool iAmOdd = ( myParticleId & 0x1 );
	int increment = SELECT( -1, +1, iAmOdd );
	int foundCount = 0;
	bool loop = ( foundCount < spaceLeft ) && ( particleCountThisCell > 0 );
	int i = 0;

	while( loop ){
		int offset = ( particleOffset + i * increment ) % particleCountThisCell;
		i++;
		offset = SELECT( offset, offset + particleCountThisCell, ( offset < 0 ) );
		int neighborParticleId = baseParticleId + offset;
		int myOffset = NEIGHBOR_COUNT - spaceLeft + foundCount;

		int thisFoundCount = considerParticle( searchCell_, neighborParticleId, position_,
			myParticleId, sortedPosition, gridCellIndex, neighborMap, myOffset, 
			h, simulationScale );

		foundCount += ( thisFoundCount == FOUND_ONE_NEIGHBOR );
		bool tooMany = ( i >= potentialNeighbors - 1);
		loop = !tooMany && ( foundCount < spaceLeft );
	}//while
	return foundCount;
}


int4 cellFactors( 
				 float4 position,
				 float xmin,
				 float ymin,
				 float zmin,
				 float hashGridCellSizeInv
				 )
{
	int4 result;
	result.x = (int)( position.x * hashGridCellSizeInv );
	result.y = (int)( position.y * hashGridCellSizeInv );
	result.z = (int)( position.z * hashGridCellSizeInv );
	return result;
}





__kernel void findNeighbors(
	__global uint * gridCellIndexFixedUp,
	__global float16 * sortedPosition16,
	int gridCellCount,
	int gridCellsX,
	int gridCellsY,
	int gridCellsZ,
	float h,
	float hashGridCellSize,
	float hashGridCellSizeInv,
	float simulationScale,
	float xmin,
	float ymin,
	float zmin,
	__global float16 * neighborMap16
	)
{
	__global float4* sortedPosition = ((__global float4*)(sortedPosition16));
	__global float2* neighborMap = ((__global float2*)(neighborMap16));
	
	__global uint * gridCellIndex = gridCellIndexFixedUp;
	int id = get_global_id( 0 );
	float4 position_ = sortedPosition[ id ];
	int myCellId = (int)POSITION_CELL_ID( position_ ) & 0xffff;

	// p is the current particle position within the bounds of the hash grid
	float4 p;
	float4 p0 = (float4)( xmin, ymin, zmin, 0.0f );
	p = position_ - p0;

	// cf is the min,min,min corner of the current cell
	int4 cellFactors_ = cellFactors( position_, xmin, ymin, zmin, hashGridCellSizeInv );
	float4 cf;
	cf.x = cellFactors_.x * hashGridCellSize;
	cf.y = cellFactors_.y * hashGridCellSize;
	cf.z = cellFactors_.z * hashGridCellSize;

	// lo.A is true if the current position is in the low half of the cell for dimension A
	int4 lo;
	lo = (( p - cf ) < h );

	int4 delta;
	int4 one = (int4)( 1, 1, 1, 1 );
	delta = one + 2 * lo;

	// search up to 8 surrounding cells
	int searchCell_;
	int foundCount = 0;

	searchCell_ = myCellId;

	int	myCellIndex = gridCellIndexFixedUp[myCellId];
	int numParticlesThisCell = gridCellIndexFixedUp[myCellId+1] - myCellIndex;
	/*
	if(numParticlesThisCell > NEIGHBOR_COUNT)
	{
		foundCount += searchForNeighbors( searchCell_, gridCellIndex, position_, 
		id, sortedPosition, neighborMap, NEIGHBOR_COUNT - foundCount, 
		h, simulationScale );
		
		return;
	}
	*/
	foundCount += searchForNeighbors( searchCell_, gridCellIndex, position_, 
		id, sortedPosition, neighborMap, NEIGHBOR_COUNT - foundCount, 
		h, simulationScale );

	if( foundCount >= NEIGHBOR_COUNT ) return;
	searchCell_ = searchCell( myCellId, delta.x, 0, 0, gridCellsX, gridCellsY, gridCellsZ, gridCellCount );
	foundCount += searchForNeighbors( searchCell_, gridCellIndex, position_, 
		id, sortedPosition, neighborMap, NEIGHBOR_COUNT - foundCount, 
		h, simulationScale );

	if( foundCount >= NEIGHBOR_COUNT ) return;
	searchCell_ = searchCell( myCellId, 0, delta.y, 0, gridCellsX, gridCellsY, gridCellsZ, gridCellCount );
	foundCount += searchForNeighbors( searchCell_, gridCellIndex, position_, 
		id, sortedPosition, neighborMap, NEIGHBOR_COUNT - foundCount, 
		h, simulationScale );

	if( foundCount >= NEIGHBOR_COUNT ) return;
	searchCell_ = searchCell( myCellId, 0, 0, delta.z, gridCellsX, gridCellsY, gridCellsZ, gridCellCount );
	foundCount += searchForNeighbors( searchCell_, gridCellIndex, position_, 
		id, sortedPosition, neighborMap, NEIGHBOR_COUNT - foundCount, 
		h, simulationScale );

	if( foundCount >= NEIGHBOR_COUNT ) return;
	searchCell_ = searchCell( myCellId, delta.x, delta.y, 0, gridCellsX, gridCellsY, gridCellsZ, gridCellCount );
	foundCount += searchForNeighbors( searchCell_, gridCellIndex, position_, 
		id, sortedPosition, neighborMap, NEIGHBOR_COUNT - foundCount, 
		h, simulationScale );

	if( foundCount >= NEIGHBOR_COUNT ) return;
	searchCell_ = searchCell( myCellId, delta.x, 0, delta.z, gridCellsX, gridCellsY, gridCellsZ, gridCellCount );
	foundCount += searchForNeighbors( searchCell_, gridCellIndex, position_, 
		id, sortedPosition, neighborMap, NEIGHBOR_COUNT - foundCount, 
		h, simulationScale );

	if( foundCount >= NEIGHBOR_COUNT ) return;
	searchCell_ = searchCell( myCellId, 0, delta.y, delta.z, gridCellsX, gridCellsY, gridCellsZ, gridCellCount );
	foundCount += searchForNeighbors( searchCell_, gridCellIndex, position_, 
		id, sortedPosition, neighborMap, NEIGHBOR_COUNT - foundCount, 
		h, simulationScale );

	if( foundCount >= NEIGHBOR_COUNT ) return;
	searchCell_ = searchCell( myCellId, delta.x, delta.y, delta.z, gridCellsX, gridCellsY, gridCellsZ, gridCellCount );
	foundCount += searchForNeighbors( searchCell_, gridCellIndex, position_, 
		id, sortedPosition, neighborMap, NEIGHBOR_COUNT - foundCount, 
		h, simulationScale );

}





int cellId( 
		   int4 cellFactors_,
		   int gridCellsX,
		   int gridCellsY,
		   int gridCellsZ
		   )
{
	int cellId_ = cellFactors_.x + cellFactors_.y * gridCellsX
		+ cellFactors_.z * gridCellsX * gridCellsY;
	return cellId_;
}



__kernel void hashParticles(
							__global float4 * position,
							int gridCellsX,
							int gridCellsY,
							int gridCellsZ,
							float hashGridCellSizeInv,
							float xmin,
							float ymin,
							float zmin,
							__global uint2 * particleIndex
							)
{
	int id = get_global_id( 0 );
	if( id >= PARTICLE_COUNT ){
		uint2 result;
		int gridCellCount = gridCellsX * gridCellsY * gridCellsZ;
		PI_CELL_ID( result ) = gridCellCount + 1;
		PI_SERIAL_ID( result ) = id;
		particleIndex[ id ] = result;
		return;
	}

	float4 _position = position[ id ];
	int4 cellFactors_ = cellFactors( _position, xmin, ymin, zmin, hashGridCellSizeInv ); 
	int cellId_ = cellId( cellFactors_, gridCellsX, gridCellsY, gridCellsZ ) & 0xffff; // truncate to low 16 bits
	uint2 result;
	PI_CELL_ID( result ) = cellId_;
	PI_SERIAL_ID( result ) = id;
	particleIndex[ id ] = result;
}







__kernel void indexPostPass(
							__global uint * gridCellIndex,
							int gridCellCount,
							__global uint * gridCellIndexFixedUp
							)
{
	int id = get_global_id( 0 );
	if( id <= gridCellCount ){
		int idx = id;
		int cellId = NO_CELL_ID;
		bool loop;
		do{
			cellId = gridCellIndex[ idx++ ];
			loop = cellId == NO_CELL_ID && idx <= gridCellCount;
		}while( loop );
		gridCellIndexFixedUp[ id ] = cellId;
	}
}



__kernel void indexx(
					 __global uint2 * particleIndex,
					 int gridCellCount,
					 __global uint * gridCellIndex
					 )
{
	int id = get_global_id( 0 );
	if( id > gridCellCount  ){
		return;
	}

	if( id == gridCellCount ){
		// add the nth+1 index value
		gridCellIndex[ id ] = PARTICLE_COUNT;
		return;
	}		
	if( id == 0 ){
		gridCellIndex[ id ] = 0;
		return;
	}

	// binary search for the starting position in sortedParticleIndex
	int low = 0;
	int high = PARTICLE_COUNT - 1;
	bool converged = false;

	int cellIndex = NO_PARTICLE_ID;
	while( !converged ){
		if( low > high ){
			converged = true;
			cellIndex = NO_PARTICLE_ID;
			continue;
		}

		int idx = ( high - low ) * 0.5f + low;
		uint2 sample = particleIndex[ idx ];
		int sampleCellId = PI_CELL_ID( sample );
		bool isHigh = ( sampleCellId > id );
		high = SELECT( high, idx - 1, isHigh );
		bool isLow = ( sampleCellId < id );
		low = SELECT( low, idx + 1, isLow );
		bool isMiddle = !( isHigh || isLow );

		uint2 zero2 = (uint2)( 0, 0 );
		uint2 sampleMinus1;
		int sampleM1CellId = 0;
		bool zeroCase = ( idx == 0 && isMiddle );
		sampleMinus1 = SELECT( particleIndex[ idx - 1 ], zero2, zeroCase );
		sampleM1CellId = SELECT( PI_CELL_ID( sampleMinus1 ), -1, zeroCase );
		bool convergedCondition = isMiddle && ( zeroCase || sampleM1CellId < sampleCellId );
		converged = convergedCondition;
		cellIndex = SELECT( cellIndex, idx, convergedCondition );
		high = SELECT( high, idx - 1, ( isMiddle && !convergedCondition ) );
	}//while

	gridCellIndex[ id ] = cellIndex;
}



void handleBoundaryConditions(
							  float4 position,
							  float4 * newVelocity,
							  float timeStep,
							  float4 * newPosition,
							  float xmin,
							  float xmax,
							  float ymin,
							  float ymax,
							  float zmin,
							  float zmax,
							  float damping
							  )
{
	if( (*newPosition).x < xmin ){
		float intersectionDistance = -position.x / (*newVelocity).x;
		float4 intersection = position + intersectionDistance * *newVelocity;
		float4 normal = (float4)( 1, 0, 0, 0 );
		float4 reflection = *newVelocity - 2.0f * DOT( *newVelocity, normal ) * normal;
		float remaining = timeStep - intersectionDistance;
		position = intersection;
		*newVelocity = reflection;
		*newPosition = intersection + remaining * damping * reflection;
	}
	else if( (*newPosition).x > xmax ){
		float intersectionDistance = ( xmax - position.x ) / (*newVelocity).x;
		float4 intersection = position + intersectionDistance * *newVelocity;
		float4 normal = (float4)( -1, 0, 0, 0 );
		float4 reflection = *newVelocity - 2.0f * DOT( *newVelocity, normal ) * normal;
		float remaining = timeStep - intersectionDistance;
		position = intersection;
		*newVelocity = reflection;
		*newPosition = intersection + remaining * damping * reflection;
	}

	if( (*newPosition).y < ymin ){
		float intersectionDistance = -position.y / (*newVelocity).y;
		float4 intersection = position + intersectionDistance * *newVelocity;
		float4 normal = (float4)( 0, 1, 0, 0 );
		float4 reflection = *newVelocity - 2.0f * DOT( *newVelocity, normal ) * normal;
		float remaining = timeStep - intersectionDistance;
		position = intersection;
		*newVelocity = reflection;
		*newPosition = intersection + remaining * damping * reflection;
	}
	else if( (*newPosition).y > ymax ){
		float intersectionDistance = ( ymax - position.y ) / (*newVelocity).y;
		float4 intersection = position + intersectionDistance * *newVelocity;
		float4 normal = (float4)( 0, -1, 0, 0 );
		float4 reflection = *newVelocity - 2.0f * DOT( *newVelocity, normal ) * normal;
		float remaining = timeStep - intersectionDistance;
		position = intersection;
		*newVelocity = reflection;
		*newPosition = intersection + remaining * damping * reflection;
	}

	if( (*newPosition).z < zmin ){
		float intersectionDistance = -position.z / (*newVelocity).z;
		float4 intersection = position + intersectionDistance * *newVelocity;
		float4 normal = (float4)( 0, 0, 1, 0 );
		float4 reflection = *newVelocity - 2.0f * DOT( *newVelocity, normal ) * normal;
		float remaining = timeStep - intersectionDistance;
		position = intersection;
		*newVelocity = reflection;
		*newPosition = intersection + remaining * damping * reflection;
	}
	else if( (*newPosition).z > zmax ){
		float intersectionDistance = ( zmax - position.z ) / (*newVelocity).z;
		float4 intersection = position + intersectionDistance * *newVelocity;
		float4 normal = (float4)( 0, 0, -1, 0 );
		float4 reflection = *newVelocity - 2.0f * DOT( *newVelocity, normal ) * normal;
		float remaining = timeStep - intersectionDistance;
		position = intersection;
		*newVelocity = reflection;
		*newPosition = intersection + remaining * damping * reflection;
	}

}



__kernel void integrate(
						__global float4 * acceleration,
						__global float4 * sortedPosition,
						__global float4 * sortedVelocity,
						float gravity_x,
						float gravity_y,
						float gravity_z,
						float simulationScaleInv,
						float timeStep,
						float xmin,
						float xmax,
						float ymin,
						float ymax,
						float zmin,
						float zmax,
						float damping,
						__global float4* position,
						__global float4* velocity
						)
{
	int id = get_global_id( 0 );
	float4 acceleration_ = acceleration[ id ];
	float4 position_ = sortedPosition[ id ];
	float4 velocity_ = sortedVelocity[ id ];

	// apply external forces
	float4 gravity = (float4)( gravity_x, gravity_y, gravity_z, 0.0f );
	acceleration_ += gravity;

	// Semi-implicit Euler integration 
	float4 newVelocity_ = velocity_ + timeStep * acceleration_;
	float posTimeStep = timeStep * simulationScaleInv; 
	float4 newPosition_ = position_ + posTimeStep * newVelocity_;

	handleBoundaryConditions( position_, &newVelocity_, posTimeStep, &newPosition_,
		xmin, xmax, ymin, ymax, zmin, zmax, damping );

	newPosition_.w = 1.0f; // homogeneous coordinate for rendering
	velocity[ id ] = newVelocity_;
	position[ id ] = newPosition_;
}





__kernel void sortPostPass(
						   __global uint2 * particleIndex,
						   __global float4 * position,
						   __global float4 * velocity,
						   __global float4 * sortedPosition,
						   __global float4 * sortedVelocity,
						   __global float* lifetimes,
						   __global float* sortedLifetimes
						   )
{
	int id = get_global_id( 0 );
	uint2 spi = particleIndex[ id ];
	int serialId = PI_SERIAL_ID( spi );
	int cellId = PI_CELL_ID( spi );
	float4 position_ = position[ serialId ];
	POSITION_CELL_ID( position_ ) = (float)cellId;
	float4 velocity_ = velocity[ serialId ];
	float lifetime_ = lifetimes[ serialId ];
	sortedVelocity[ id ] = velocity_;
	sortedPosition[ id ] = position_;
	sortedLifetimes[ id ] = lifetime_;
}

__kernel void radixFillShiftBuffer(
	const __global uint2* particleIndex,
	__global uint* shiftBuff,
	volatile __global uint* workgroupOneCounts,
	const uint mask,
	volatile __global uint* totalOnes
	)
{
	int gid = get_global_id(0)*8; 
	int lid = get_local_id(0);
	int wgid = gid / get_local_size(0);
	
	uint curr = 0;
	uint total = 0;
	
	uint16 pins = ((__global uint16*)(particleIndex+gid))[0];
	uint8 shifts;

	shifts.s0 = (pins.s0 & mask) > 0;
	
	shifts.s1 = (pins.s2 & mask) > 0;
	
	shifts.s2 = (pins.s4 & mask) > 0;
	
	shifts.s3 = (pins.s6 & mask) > 0;
	
	shifts.s4 = (pins.s8 & mask) > 0;
	
	shifts.s5 = (pins.sa & mask) > 0;
	
	shifts.s6 = (pins.sc & mask) > 0;
	
	shifts.s7 = (pins.se & mask) > 0;
	
	
	((__global uint8*)(shiftBuff+gid))[0] = shifts;
	int totalO = shifts.s0+shifts.s1+shifts.s2+shifts.s3+shifts.s4+shifts.s5+shifts.s6+shifts.s7;
	atomic_add(workgroupOneCounts+wgid, totalO);
	atomic_add(totalOnes, totalO);
}

__kernel void radixClearBuffers(
	__global uint* workgroupOneCounts,
	__global uint* totalOnes
	)
{
	int gid = get_global_id(0);
	workgroupOneCounts[gid] = 0;
	if(gid==0)
		totalOnes[0] = 0;
}

__kernel void radixParseShiftBuffer(
	__global uint* workgroupOneCounts,
	__global uint* totalOnes
	)
{
	int gid = get_global_id(0);
	//atomic_add(totalOnes, workgroupOneCounts[gid]);
}

__kernel void radixFillParticleIndexOut(
	const __global uint2* particleIndexIn,
	__global uint2* particleIndexOut,
	const __global uint* shiftBuff,
	const __global uint* workgroupOneCounts,
	__global uint* totalOnes
	)
{
	__local uint shifts[8192];
	__local uint wgoc[64];
	int gSize = get_global_size(0)*8;
	int lSize = get_local_size(0);
	int gid = get_global_id(0)*8;
	int lid = get_local_id(0);
	int wgid = gid / lSize;
	int numGroups = gSize/lSize;
	int sid = gid%8192;
	int wgStart = (wgid%8)*1024;
	event_t bar = async_work_group_copy(shifts, shiftBuff+get_group_id(0)*8192, 8192, 0);
	event_t bar2 = async_work_group_copy(wgoc, workgroupOneCounts, 64, 0);
	wait_group_events(1,&bar);
	wait_group_events(1,&bar2);
	uint8 pShifts = ((__local uint8*)(shifts+sid))[0];
	
	
	int onesToLeft = 0;

	for(int i=wgStart; i<sid; i++)
		onesToLeft += shifts[i];
	
	// otl = wgoc[wgid] - otr;
	
	for(int i=0; i<=wgid; i++)
		onesToLeft += wgoc[i];

	onesToLeft -= wgoc[wgid];
	//int totalZeroes = gSize - totalOnes[0];
	//int zeroesToLeft = gid - onesToLeft;
	//int zeroesOnMyIndex = 1 - pShifts.s0;
	//int zeroesToRight = totalZeroes - zeroesToLeft;
	
	int zeroesToRight = gSize - totalOnes[0] - gid + onesToLeft;
	
	int oz[2];
	oz[0] = -onesToLeft;
	oz[1] = zeroesToRight;
	
	uint16 pin = ((__global uint16*)(particleIndexIn+gid))[0];
	
	uint2 pinP[8];
	pinP[0] = pin.s01;
	pinP[1] = pin.s23;
	pinP[2] = pin.s45;
	pinP[3] = pin.s67;
	pinP[4] = pin.s89;
	pinP[5] = pin.sab;
	pinP[6] = pin.scd;
	pinP[7] = pin.sef;
	
	for(int i=0; i<8; i++)
	{
		int a = shifts[sid++];
		int b = (a+1)%2;
		particleIndexOut[gid++ + oz[a]] = pinP[i];
		oz[b]--;
	}
	
	/*
	int a = pShifts.s0;
	int b = (a==0)?1:0;
	particleIndexOut[gid++ + oz[a]] = pin.s01;
	oz[b]--;
	
	a = pShifts.s1;
	b = (a==0)?1:0;
	particleIndexOut[gid++ + oz[a]] = pin.s23;
	oz[b]--;
	
	a = pShifts.s2;
	b = (a==0)?1:0;
	particleIndexOut[gid++ + oz[a]] = pin.s45;
	oz[b]--;
	
	a = pShifts.s3;
	b = (a==0)?1:0;
	particleIndexOut[gid++ + oz[a]] = pin.s67;
	oz[b]--;
	
	a = pShifts.s4;
	b = (a==0)?1:0;
	particleIndexOut[gid++ + oz[a]] = pin.s89;
	oz[b]--;
	
	a = pShifts.s5;
	b = (a==0)?1:0;
	particleIndexOut[gid++ + oz[a]] = pin.sab;
	oz[b]--;
	
	a = pShifts.s6;
	b = (a==0)?1:0;
	particleIndexOut[gid++ + oz[a]] = pin.scd;
	oz[b]--;
	
	a = pShifts.s7;
	//b = (a+1)%2;
	particleIndexOut[gid++ + oz[a]] = pin.sef;
	//oz[b]--;
	*/
}

float4 minCornerOfCell(
	int cellID,
	int gridCellsX,
	int gridCellsY,
	int gridCellsZ,
	float hashGridCellSize
	)
{
	float4 ret;
	ret.x = cellID % gridCellsX;
	ret.y = ((cellID - ((int)ret.x))/gridCellsX)%gridCellsY;
	ret.z = (cellID - ((int)ret.x) - ((int)ret.y)*gridCellsX) / gridCellsY;
	ret.xyz = ret.xyz * hashGridCellSize;
	ret.w = cellID; 
	return ret;
}

__kernel void cycleParticles(
	__global float4* position,
	__global float4* velocity,
	int flowStartCell,
	int flowEndCell,
	int gridCellsX,
	int gridCellsY,
	int gridCellsZ,
	float hashGridCellSize,
	float4 sourceVelocity,
	float endSize,
	float endMinX,
	float endMinZ,
	float timeStep,
	__global float* lifetimes,
	__global float* sortedLifetimes
	)
{
	int gid = get_global_id(0);
	int gSize = get_global_size(0);
	int lid = get_local_id(0);
	int lSize = get_local_size(0);
	float4 pos = position[gid];
	lifetimes[gid] = sortedLifetimes[gid] + timeStep;
	if(pos.z > endMinZ && pos.z < endMinZ+endSize
		&& pos.x > endMinX && pos.x < endMinX+endSize
		&& pos.y < 15.0f)
	{
		uint seed1 = ((uint)(pos.x*pos.y*pos.z));
		uint seed2 = ((uint)(pos.x+pos.y+pos.z));
		float rand1 = (float)(myRandom(seed1, PARTICLE_COUNT))/PARTICLE_COUNT;
		float rand2 = (float)(myRandom(seed2, PARTICLE_COUNT))/PARTICLE_COUNT;
		float4 newPos = (float4)(18.0, 25.0, 50.0, 0.0);
		newPos += (float4)(30.0*rand1, 15.0*rand2, 0.0, 0.0);
		position[gid] = newPos;
		velocity[gid] = sourceVelocity;
		lifetimes[gid] = 0.0;
	}
}