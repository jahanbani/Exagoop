#ifndef PARTICLEBASIS_H_
#define PARTICLEBASIS_H_

#include<constants.H>
#include <AMReX.H>
#include<mpm_particle_container.H>

AMREX_GPU_DEVICE AMREX_FORCE_INLINE
amrex::Real spherical_gaussian(amrex::Real xi[3],amrex::Real xp[3],amrex::Real r0)
{
  amrex::Real f;
  amrex::Real r2;

  r2=(xi[0]-xp[0])*(xi[0]-xp[0])+(xi[1]-xp[1])*(xi[1]-xp[1])+(xi[2]-xp[2])*(xi[2]-xp[2]);
  f=std::exp(-r2/(r0*r0))*std::pow(PI,-1.5)*std::pow(r0,-3.0);

  return(f);
}

AMREX_GPU_DEVICE AMREX_FORCE_INLINE
amrex::Real box_kernel(amrex::Real xi[3],amrex::Real xp[3],amrex::Real r0)
{
    amrex::Real r2=(xi[0]-xp[0])*(xi[0]-xp[0])+(xi[1]-xp[1])*(xi[1]-xp[1])+(xi[2]-xp[2])*(xi[2]-xp[2]);
    amrex::Real f=(r2<r0*r0)?1.0:0.0;

    return(f);
}

AMREX_GPU_DEVICE AMREX_FORCE_INLINE
amrex::Real hat1d(amrex::Real xi,amrex::Real xp,amrex::Real hatsize)
{
    amrex::Real zeta=xi-xp;
    amrex::Real halfhatsize=hatsize*half;
    amrex::Real funcval=zero;
    amrex::Real maxval=one/halfhatsize;
    amrex::Real slope=four/(hatsize*hatsize);

    if(amrex::Math::abs(zeta) < halfhatsize)
    {
        if(zeta>0.0)
        {
            funcval = -slope*zeta + maxval;       
        }
        else
        {
            funcval =  slope*zeta + maxval;
        }
    }
    return(funcval);
}

AMREX_GPU_DEVICE AMREX_FORCE_INLINE
amrex::Real hat3d(amrex::Real xi[AMREX_SPACEDIM], amrex::Real xp[AMREX_SPACEDIM],
                  amrex::Real hatsize[AMREX_SPACEDIM])
{
    amrex::Real funcval=one;
    funcval *= hat1d(xi[XDIR],xp[XDIR],hatsize[XDIR]);
    funcval *= hat1d(xi[YDIR],xp[YDIR],hatsize[YDIR]);
    funcval *= hat1d(xi[ZDIR],xp[ZDIR],hatsize[ZDIR]);

    return(funcval);
}

AMREX_GPU_DEVICE AMREX_FORCE_INLINE
amrex::Real bilin_interp(amrex::Real xp[AMREX_SPACEDIM], int i, int j, int k, 
                         const amrex::GpuArray<amrex::Real, AMREX_SPACEDIM> plo, 
                         const amrex::GpuArray<amrex::Real, AMREX_SPACEDIM> dx,
                         amrex::Array4<amrex::Real> nodaldata,
                         int comp)
{

    amrex::Real alpha,beta,gama;
    amrex::Real value=zero;

    alpha = (xp[XDIR]-(plo[XDIR]+i*dx[XDIR]))/dx[XDIR];
    beta  = (xp[YDIR]-(plo[YDIR]+j*dx[YDIR]))/dx[YDIR];
    gama  = (xp[ZDIR]-(plo[ZDIR]+k*dx[ZDIR]))/dx[ZDIR];


    value += (1.0-alpha) * (1-beta) * (1-gama) * nodaldata(i,j,k,comp);
    value += alpha       * (1-beta) * (1-gama) * nodaldata(i+1,j,k,comp);
    value += (1.0-alpha) * beta     * (1-gama) * nodaldata(i,j+1,k,comp);
    value += alpha       * beta     * (1-gama) * nodaldata(i+1,j+1,k,comp);

    value += (1.0-alpha) * (1-beta)  * gama  * nodaldata(i,j,k+1,comp);
    value += alpha       * (1-beta)  * gama  * nodaldata(i+1,j,k+1,comp);
    value += (1.0-alpha) * beta      * gama  * nodaldata(i,j+1,k+1,comp);
    value += alpha       * beta      * gama  * nodaldata(i+1,j+1,k+1,comp);

    return(value);
}


AMREX_GPU_DEVICE AMREX_FORCE_INLINE
amrex::Real cubicspline_1d_der(int shapefunctiontype,amrex:: Real zi)
{
    amrex::Real value=0.0;
    if(shapefunctiontype==1)	//Boundary node
    {
        if(zi>=-2 && zi<=-1)
        {
            value=zi*zi/2.0+2*zi+2.0;
        }
        else if(zi>=-1 && zi<=0)
        {
            value=-zi*zi/2.0+1.0;
        }
        else if(zi>=0 && zi<=1)
        {
            value=zi*zi/2.0-1.0;
        }
        else if(zi>=1 && zi<=2.0)
        {
            value=-zi*zi/2.0+2*zi-2.0;
        }
    }
    else if(shapefunctiontype==2)	//Near Boundary node
    {
        if(zi>=-1 && zi<=0)
		{
			value=-zi*zi-2.0*zi;
		}
		else if(zi>=0 && zi<=1)
		{
			value=3.0*zi*zi/2.0-2.0*zi;
		}
		else if(zi>=1 && zi<=2.0)
		{
			value=-zi*zi/2.0+2*zi-2.0;
		}
	}
	else if(shapefunctiontype==3)	//Interior node
	{
		if(zi>=-2 && zi<=-1)
		{
			value=zi*zi/2.0+2*zi+2.0;
		}
		else if(zi>=-1 && zi<=0)
		{
			value=-3.0*zi*zi/2.0-2.0*zi;
		}
		else if(zi>=0 && zi<=1)
		{
			value=3.0*zi*zi/2.0-2.0*zi;
		}
		else if(zi>=1 && zi<=2.0)
		{
			value=-zi*zi/2.0+2*zi-2.0;
		}
	}
	else if(shapefunctiontype==4)	//Interior node
	{
		if(zi>=-2 && zi<=-1)
		{
			value=zi*zi/2.0+2*zi+2.0;
		}
		else if(zi>=-1 && zi<=0)
		{
			value=-3.0*zi*zi/2.0-2.0*zi;
		}
		else if(zi>=0 && zi<=1)
		{
			value=zi*zi-2.0*zi;
		}
	}
	else
	{
#ifndef AMREX_USE_GPU
		amrex::Print()<<"\n Shapefunction = "<<shapefunctiontype;
#endif
		amrex::Abort("\n Incorrect shape function type in cubicspline_1d_der");
	}
	return value;

}

AMREX_GPU_DEVICE AMREX_FORCE_INLINE
        amrex::Real cubicspline_1d(int shapefunctiontype,amrex:: Real zi,int dir)
{
	amrex::Real value=0.0;
	if(shapefunctiontype==1)	//Boundary node
	{
		if(zi>=-2 && zi<=-1)
		{
			value=zi*zi*zi/6.0+zi*zi+2.0*zi+4.0/3.0;
		}
		else if(zi>=-1 && zi<=0)
		{
			value=-zi*zi*zi/6.0+zi+1.0;
		}
		else if(zi>=0 && zi<=1)
		{
			value=zi*zi*zi/6.0-zi+1.0;
		}
		else if(zi>=1 && zi<=2.0)
		{
			value=-zi*zi*zi/6.0+zi*zi-2.0*zi+4.0/3.0;
		}
		else
		{
			#ifndef AMREX_USE_GPU
			amrex::Print()<<"\n Error in shape func 1: "<<zi;
			#endif

		}
	}
	else if(shapefunctiontype==2)	//Near Boundary node
	{
		if(zi>=-1 && zi<=0)
		{
			value=-zi*zi*zi/3.0-zi*zi+2.0/3.0;
		}
		else if(zi>=0 && zi<=1)
		{
			value=zi*zi*zi/2.0-zi*zi+2.0/3.0;
		}
		else if(zi>=1 && zi<=2.0)
		{
			value=-zi*zi*zi/6.0+zi*zi-2.0*zi+4.0/3.0;
		}
		else
		{
			#ifndef AMREX_USE_GPU
			amrex::Print()<<"\n Error in shape func 2: "<<zi<<" dir = "<<dir;
			#endif

		}
	}
	else if(shapefunctiontype==3)	//Interior node
	{
		if(zi>=-2 && zi<=-1)
		{
			value=zi*zi*zi/6.0+zi*zi+2.0*zi+4.0/3.0;
		}
		else if(zi>=-1 && zi<=0)
		{
			value=-zi*zi*zi/2.0-zi*zi+2.0/3.0;
		}
		else if(zi>=0 && zi<=1)
		{
			value=zi*zi*zi/2.0-zi*zi+2.0/3.0;
		}
		else if(zi>=1 && zi<=2.0)
		{
			value=-zi*zi*zi/6.0+zi*zi-2.0*zi+4.0/3.0;
		}
		else
		{
			#ifndef AMREX_USE_GPU
			amrex::Print()<<"\n Error in shape func 3: "<<zi;
			#endif
		}
	}
	else if(shapefunctiontype==4)	//Interior node
	{
		if(zi>=-2 && zi<=-1)
		{
			value=zi*zi*zi/6.0+zi*zi+2.0*zi+4.0/3.0;
		}
		else if(zi>=-1 && zi<=0)
		{
			value=-zi*zi*zi/2.0-zi*zi+2.0/3.0;
		}
		else if(zi>=0 && zi<=1)
		{
			value=zi*zi*zi/3.0-zi*zi+2.0/3.0;
		}
		else
		{
			#ifndef AMREX_USE_GPU
			amrex::Print()<<"\n Error in shape func 4: "<<zi<<" dir= "<<dir;
			#endif
		}
	}
	else
	{
#ifndef AMREX_USE_GPU
		amrex::Print()<<"\n Shapefunction = "<<shapefunctiontype;
#endif
		amrex::Abort("\n Incorrect shape function type in cubicspline_1d");
	}
	return value;

}

AMREX_GPU_DEVICE AMREX_FORCE_INLINE
        amrex::Real cubic_interp(amrex::Real xp[AMREX_SPACEDIM],
        		int i, int j, int k,
        		int lmin,int mmin,int nmin,int lmax,int mmax, int nmax,
                const amrex::GpuArray<amrex::Real, AMREX_SPACEDIM> plo,
                const amrex::GpuArray<amrex::Real, AMREX_SPACEDIM> dx,
                amrex::Array4<amrex::Real> nodaldata,
				int comp,const int* lo,const int* hi)
{

    amrex::Real rx,ry,rz;
    amrex::Real lval,mval,nval;
    amrex::Real value=zero;
    int shapetypex,shapetypey,shapetypez;

    for(int n=nmin;n<nmax;n++)
    {
    	for(int m=mmin;m<mmax;m++)
    	{
    		for(int l=lmin;l<lmax;l++)
    		{
    			if(i+l==lo[0]) shapetypex=1;
    			else if(i+l==lo[0]+1) shapetypex=2;
    			else if(i+l==hi[0]+1) shapetypex=1;
    			else if(i+l==hi[0]) shapetypex=4;
    			else shapetypex=3;

    			if(j+m==lo[1]) shapetypey=1;
    			else if(j+m==lo[1]+1) shapetypey=2;
    			else if(j+m==hi[1]+1) shapetypey=1;
    			else if(j+m==hi[1]) shapetypey=4;
    			else shapetypey=3;

    			if(k+n==lo[2]) shapetypez=1;
    			else if(k+n==lo[2]+1) shapetypez=2;
    			else if(k+n==hi[2]+1) shapetypez=1;
    			else if(k+n==hi[2]) shapetypez=4;
    			else shapetypez=3;

    			rx = (xp[XDIR]-(plo[XDIR]+(i+l)*dx[XDIR]))/dx[XDIR];
    			ry  = (xp[YDIR]-(plo[YDIR]+(j+m)*dx[YDIR]))/dx[YDIR];
    			rz  = (xp[ZDIR]-(plo[ZDIR]+(k+n)*dx[ZDIR]))/dx[ZDIR];

    			//amrex::Print()<<"\n dx = "<<dx[XDIR]<<" "<<dx[YDIR]<<" "<<dx[ZDIR];

    			lval=cubicspline_1d(shapetypex,rx,0);
    			mval=cubicspline_1d(shapetypey,ry,1);
    			nval=cubicspline_1d(shapetypez,rz,2);
    			value += lval*mval*nval* nodaldata(i+l,j+m,k+n,comp);

    		}
    	}
    }

    return(value);
}


AMREX_GPU_DEVICE AMREX_FORCE_INLINE
        amrex::Real basisval(int l,int m,int n, int i, int j, int k,
               amrex::Real xp[AMREX_SPACEDIM], 
                const amrex::GpuArray<amrex::Real, AMREX_SPACEDIM> plo, 
                const amrex::GpuArray<amrex::Real, AMREX_SPACEDIM> dx,
				GpuArray <int, AMREX_SPACEDIM> order_scheme_directional,
				GpuArray <int, AMREX_SPACEDIM> periodic,
				const int *lo,
				const int *hi)
{
    amrex::Real lval,mval,nval;
    amrex::Real rx,ry,rz;

    int order_scheme_dir[]={AMREX_D_DECL(order_scheme_directional[XDIR],order_scheme_directional[YDIR],order_scheme_directional[ZDIR])};

    lval=0.0;
    mval=0.0;
    nval=0.0;
    
    int shapefunctype;

    rx 	= (xp[XDIR]-(plo[XDIR]+i*dx[XDIR]))/dx[XDIR];
    ry  = (xp[YDIR]-(plo[YDIR]+j*dx[YDIR]))/dx[YDIR];
    rz  = (xp[ZDIR]-(plo[ZDIR]+k*dx[ZDIR]))/dx[ZDIR];

    if(order_scheme_dir[0]==1)
    {
    	lval=(l==0)?(one-rx):rx;
    }
    else
    {
    	if(periodic[0]==0)
    	{
    		shapefunctype = ((i+l)==lo[XDIR] || (i+l)==hi[XDIR]+1)?1:((i+l==lo[XDIR]+1)?2:((i+l==hi[XDIR]))?4:3);
    		rx = (xp[XDIR]-(plo[XDIR]+(i+l)*dx[XDIR]))/dx[XDIR];
    		lval=cubicspline_1d(shapefunctype,rx,0);
    	}
    	else
    	{
    		shapefunctype = 3;
    		rx = (xp[XDIR]-(plo[XDIR]+(i+l)*dx[XDIR]))/dx[XDIR];
    		lval=cubicspline_1d(shapefunctype,rx,0);
    	}
    }

    if(order_scheme_dir[1]==1)
    {
    	mval=(m==0)?(one-ry):ry;
    }
    else
    {
    	if(periodic[1]==0)
    	{
    		shapefunctype = ((j+m)==lo[YDIR] || (j+m)==hi[YDIR]+1)?1:((j+m==lo[YDIR]+1)?2:((j+m==hi[YDIR]))?4:3);
    		ry = (xp[YDIR]-(plo[YDIR]+(j+m)*dx[YDIR]))/dx[YDIR];
    		mval=cubicspline_1d(shapefunctype,ry,1);

    	}
    	else
    	{
    		shapefunctype = 3;
    		ry = (xp[YDIR]-(plo[YDIR]+(j+m)*dx[YDIR]))/dx[YDIR];
    		mval=cubicspline_1d(shapefunctype,ry,1);
    	}
    }

    if(order_scheme_dir[2]==1)
    {
    	nval=(n==0)?(one-rz):rz;
    }
    else
    {
    	if(periodic[2]==0)
    	{
    		shapefunctype = ((k+n)==lo[ZDIR] || (k+n)==hi[ZDIR]+1)?1:((k+n==lo[ZDIR]+1)?2:((k+n==hi[ZDIR]))?4:3);
    		rz = (xp[ZDIR]-(plo[ZDIR]+(k+n)*dx[ZDIR]))/dx[ZDIR];
    		nval=cubicspline_1d(shapefunctype,rz,2);
    	}
        else
        {
        	shapefunctype = 3;
        	rz = (xp[ZDIR]-(plo[ZDIR]+(k+n)*dx[ZDIR]))/dx[ZDIR];
        	nval=cubicspline_1d(shapefunctype,rz,2);
        }
    }
    return(lval*mval*nval);
}

AMREX_GPU_DEVICE AMREX_FORCE_INLINE
        amrex::Real basisvalder(int dir, int l,int m,int n, int i, int j, int k, 
                amrex::Real xp[AMREX_SPACEDIM], 
                const amrex::GpuArray<amrex::Real, AMREX_SPACEDIM> plo, 
                const amrex::GpuArray<amrex::Real, AMREX_SPACEDIM> dx,
				GpuArray <int, AMREX_SPACEDIM> order_scheme_directional,
				GpuArray <int, AMREX_SPACEDIM> periodic,
				const int *lo,
				const int *hi)
{
    amrex::Real lval,mval,nval;
    amrex::Real rx,ry,rz;
    amrex::Real dxinv[AMREX_SPACEDIM];

    int shapefunctype;

    dxinv[XDIR]=one/dx[XDIR];
    dxinv[YDIR]=one/dx[YDIR];
    dxinv[ZDIR]=one/dx[ZDIR];

    rx  = (xp[XDIR]-(plo[XDIR]+i*dx[XDIR]))/dx[XDIR];
    ry  = (xp[YDIR]-(plo[YDIR]+j*dx[YDIR]))/dx[YDIR];
    rz  = (xp[ZDIR]-(plo[ZDIR]+k*dx[ZDIR]))/dx[ZDIR];

    if(order_scheme_directional[0]==1)
    {
    	lval=(l==0)?(one-rx):rx;
    }
    else if(order_scheme_directional[0]==3)
    {
    	if(periodic[0]==0)
    	{
    		shapefunctype = ((i+l)==lo[XDIR] || (i+l)==hi[XDIR]+1)?1:((i+l==lo[XDIR]+1)?2:((i+l==hi[XDIR]))?4:3);
    		rx = (xp[XDIR]-(plo[XDIR]+(i+l)*dx[XDIR]))/dx[XDIR];
    		lval=cubicspline_1d(shapefunctype,rx,0);
    	}
    	else
    	{
    		shapefunctype = 3;
    		rx = (xp[XDIR]-(plo[XDIR]+(i+l)*dx[XDIR]))/dx[XDIR];
    		lval=cubicspline_1d(shapefunctype,rx,0);
    	}
    }

    if(order_scheme_directional[1]==1)
    {
    	mval=(m==0)?(one-ry):ry;
    }
    else if(order_scheme_directional[1]==3)
    {
    	if(periodic[1]==0)
    	{
    		shapefunctype = ((j+m)==lo[YDIR] || (j+m)==hi[YDIR]+1)?1:((j+m==lo[YDIR]+1)?2:((j+m==hi[YDIR]))?4:3);
    		ry = (xp[YDIR]-(plo[YDIR]+(j+m)*dx[YDIR]))/dx[YDIR];
    		mval=cubicspline_1d(shapefunctype,ry,1);
    	}
    	else
    	{
    		shapefunctype = 3;
    		ry = (xp[YDIR]-(plo[YDIR]+(j+m)*dx[YDIR]))/dx[YDIR];
    		mval=cubicspline_1d(shapefunctype,ry,1);
    	}
    }

    if(order_scheme_directional[2]==1)
    {
    	nval=(n==0)?(one-rz):rz;
    }
    else if(order_scheme_directional[2]==3)
    {
    	if(periodic[2]==0)
    	{
    		shapefunctype = ((k+n)==lo[ZDIR] || (k+n)==hi[ZDIR]+1)?1:((k+n==lo[ZDIR]+1)?2:((k+n==hi[ZDIR]))?4:3);
    		rz = (xp[ZDIR]-(plo[ZDIR]+(k+n)*dx[ZDIR]))/dx[ZDIR];
    		nval=cubicspline_1d(shapefunctype,rz,2);
    	}
    	else
    	{
    		shapefunctype = 3;
    		rz = (xp[ZDIR]-(plo[ZDIR]+(k+n)*dx[ZDIR]))/dx[ZDIR];
    		nval=cubicspline_1d(shapefunctype,rz,2);
    	}
    }

    if(dir==0)
    {
    	if(order_scheme_directional[0]==1)
    	{
    		lval=(l==0)?-dxinv[XDIR]:dxinv[XDIR];
    	}
    	else
    	{
    		shapefunctype = ((i+l)==lo[XDIR] || (i+l)==hi[XDIR]+1)?1:((i+l==lo[XDIR]+1)?2:((i+l==hi[XDIR]))?4:3);
    		rx = (xp[XDIR]-(plo[XDIR]+(i+l)*dx[XDIR]))/dx[XDIR];
    		lval=cubicspline_1d_der(shapefunctype,rx)*dxinv[XDIR];
    	}
    }
    else if(dir==1)
    {
    	if(order_scheme_directional[1]==1)
    	{
    		mval=(m==0)?-dxinv[YDIR]:dxinv[YDIR];
    	}
    	else
    	{
    		shapefunctype = ((j+m)==lo[YDIR] || (j+m)==hi[YDIR]+1)?1:((j+m==lo[YDIR]+1)?2:((j+m==hi[YDIR]))?4:3);
    		ry = (xp[YDIR]-(plo[YDIR]+(j+m)*dx[YDIR]))/dx[YDIR];
    		mval=cubicspline_1d_der(shapefunctype,ry)*dxinv[YDIR];
    	}
    }
    else if(dir==2)
    {
    	if(order_scheme_directional[2]==1)
    	{
    		nval=(n==0)?-dxinv[ZDIR]:dxinv[ZDIR];
    	}
    	else
    	{
    		shapefunctype = ((k+n)==lo[ZDIR] || (k+n)==hi[ZDIR]+1)?1:((k+n==lo[ZDIR]+1)?2:((k+n==hi[ZDIR]))?4:3);
    		rz = (xp[ZDIR]-(plo[ZDIR]+(k+n)*dx[ZDIR]))/dx[ZDIR];
    		nval=cubicspline_1d_der(shapefunctype,rz)*dxinv[ZDIR];
    	}
    }

    return(lval*mval*nval);
}


AMREX_GPU_DEVICE AMREX_FORCE_INLINE
        void get_tensor(MPMParticleContainer::ParticleType &p,int start_index,
                amrex::Real tens[AMREX_SPACEDIM*AMREX_SPACEDIM])
{
   int ind=0;
   for(int i=0;i<AMREX_SPACEDIM;i++)
   {
      for(int j=i;j<AMREX_SPACEDIM;j++)
      {
        tens[i*AMREX_SPACEDIM+j]=p.rdata(start_index+ind);
        tens[j*AMREX_SPACEDIM+i]=p.rdata(start_index+ind);
        ind++;
      }    
   }
}

AMREX_GPU_DEVICE AMREX_FORCE_INLINE
        void get_deformation_gradient_tensor(MPMParticleContainer::ParticleType &p,int start_index,
                amrex::Real gradvp[AMREX_SPACEDIM][AMREX_SPACEDIM],amrex::Real dt)
{
	amrex::Real Lp[AMREX_SPACEDIM][AMREX_SPACEDIM];
	amrex::Real Fp[AMREX_SPACEDIM][AMREX_SPACEDIM];

	for(int i=0;i<AMREX_SPACEDIM;i++)
	{
		for(int j=0;j<AMREX_SPACEDIM;j++)
		{
			if(i==j)
			{
				Lp[i][j]=1.0+gradvp[i][j]*dt;
			}
			else
			{
				Lp[i][j]=gradvp[i][j]*dt;
			}
		}
	}

	Fp[0][0]=p.rdata(start_index+0);
	Fp[0][1]=p.rdata(start_index+1);
	Fp[0][2]=p.rdata(start_index+2);
	Fp[1][0]=p.rdata(start_index+3);
	Fp[1][1]=p.rdata(start_index+4);
	Fp[1][2]=p.rdata(start_index+5);
	Fp[2][0]=p.rdata(start_index+6);
	Fp[2][1]=p.rdata(start_index+7);
	Fp[2][2]=p.rdata(start_index+8);

	//Lp * Fp
	p.rdata(start_index+0)= Lp[0][0]*Fp[0][0]+Lp[0][1]*Fp[1][0]+Lp[0][2]*Fp[2][0];
	p.rdata(start_index+1)= Lp[0][0]*Fp[0][1]+Lp[0][1]*Fp[1][1]+Lp[0][2]*Fp[2][1];
	p.rdata(start_index+2)= Lp[0][0]*Fp[0][2]+Lp[0][1]*Fp[1][2]+Lp[0][2]*Fp[2][2];
	p.rdata(start_index+3)= Lp[1][0]*Fp[0][0]+Lp[1][1]*Fp[1][0]+Lp[1][2]*Fp[2][0];
	p.rdata(start_index+4)= Lp[1][0]*Fp[0][1]+Lp[1][1]*Fp[1][1]+Lp[1][2]*Fp[2][1];
	p.rdata(start_index+5)= Lp[1][0]*Fp[0][2]+Lp[1][1]*Fp[1][2]+Lp[1][2]*Fp[2][2];
	p.rdata(start_index+6)= Lp[2][0]*Fp[0][0]+Lp[2][1]*Fp[1][0]+Lp[2][2]*Fp[2][0];
	p.rdata(start_index+7)= Lp[2][0]*Fp[0][1]+Lp[2][1]*Fp[1][1]+Lp[2][2]*Fp[2][1];
	p.rdata(start_index+8)= Lp[2][0]*Fp[0][2]+Lp[2][1]*Fp[1][2]+Lp[2][2]*Fp[2][2];

}

AMREX_GPU_DEVICE AMREX_FORCE_INLINE
        void tensor_vector_pdt(amrex::Real tens[AMREX_SPACEDIM*AMREX_SPACEDIM],
                amrex::Real vect[AMREX_SPACEDIM],amrex::Real tensvect[AMREX_SPACEDIM])
{
    for(int i=0;i<AMREX_SPACEDIM;i++)
    {
        tensvect[i]=zero;
        for(int j=0;j<AMREX_SPACEDIM;j++)
        {
            tensvect[i] += tens[i*AMREX_SPACEDIM+j]*vect[j];
        }
    }
}



#endif
