/*
 * ORBSTEP.C: general-purpose orbit-integration routines.
 * Defines: initstep(), orbstep().
 * Requires: MBODY, body, bodyptr, Pos(), Vel(), Acc().
 *
 *  10-jun-92  Added the 'rk4' method, but this
 *             now uses VECTMATH and assumes particles are
 *             not 'interacting' - build for rotating potentials  
 *             Note that RK, PC and PC1 don't work for rotating
 *             potentials.					PJT
 */

#include "defs.h"

/*
 * ABAK0, ..., ABAK3: saved accelerations, latest to oldest.
 */

#define MCOR  (NDIM * MBODY)

local real abak0[MCOR];
local real abak1[MCOR];
local real abak2[MCOR];
local real abak3[MCOR];

local void moveaccel();

/*
 * INITSTEP: initialize the orbit-integrator.
 */

local int nstep;	/* integration step counter */

initstep(btab, nb, tptr, force)
bodyptr btab;		/* array of bodies */
int nb;			/* number of bodies */
real *tptr;		/* initial time */
proc force;		/* acceleration calculation */
{
    nstep = 0;					/* start counting steps */
    (*force)(btab, nb, *tptr);			/* compute (t-dep) force */
    moveaccel(btab, nb);			/* save resulting accel */
}

/*
 * ORBSTEP: integrate the orbit of a set of bodies.
 */

orbstep(btab, nb, tptr, force, dt, mode)
bodyptr btab;		/* array of bodies */
int nb;			/* number of bodies */
real *tptr;		/* current time */
proc force;		/* acceleration calculation */
real dt;		/* integration time step */
int mode;		/* select integration algorithm */
{
    if (mode == 0) {                            /* simplest Eulerian */
        eulerstep(btab, nb, tptr, force, dt);
        (*force)(btab, nb, *tptr);              /* compute new force */
    } else if (mode == 5) {			/* use leapfrog ? */
        leapfrogstep(btab, nb, tptr, force, dt);/* take step */
	(*force)(btab, nb, *tptr);		/* compute final force ? */
    } else if (mode == 4) {			/* use 4th order RK ? */
        rk4step(btab, nb, tptr, force, dt);	/* take RK4 step */
	(*force)(btab, nb, *tptr);		/* compute final force */
    } else if (mode == 1 || nstep < 3) {	/* RK algorithm required? */
        rkstep(btab, nb, tptr, force, dt, abak3);
						/*   take RK time-step */
	(*force)(btab, nb, *tptr);		/*   compute final force */
    } else if (mode == 2 || mode == 3) {	/* PC algorithm possible? */
        pcstep(btab, nb, tptr, force, dt);	/*   take PC time-step */
	if (mode == 2)				/*   final force requested? */
	    (*force)(btab, nb, *tptr);		/*   compute final force */
    } else if (mode == -1) {			/* use 4th order RK ? */
      epistep(btab, nb, tptr, force, dt);	/* take epicycle step */
	(*force)(btab, nb, *tptr);		/* compute final force */
    } else 
	error("orbstep: unknown mode %d\n", mode);
    moveaccel(btab, nb);			/* save final accel */
    nstep++;					/* count another time-step */
}

/*
 * RKSTEP: (some kind of) Runge-Kutta step
 */
rkstep(btab, nb, tptr, force, dt, atmp1)
bodyptr btab;		/* array of bodies */
int nb;			/* number of bodies */
real *tptr;		/* current time */
proc force;		/* acceleration calculation */
real dt;		/* integration time step */
real atmp1[];		/* scratch accelerations */
{
    bodyptr p;
    int i, k;
    register real *pptr, *vptr, *aptr;
    real dt2, dts4, dt6, dts6;
    static real atmp2[MCOR];			/* dont alloc on stack */

    dt2 = dt / 2;
    for (p = btab; p < btab+nb; p++) {		/* loop over bodies */
        pptr = Pos(p);				/*   get body coords */
	vptr = Vel(p);
	for (k = 0; k < NDIM; k++)		/*   loop over coords */
	    *pptr++ += dt2 * (*vptr++);		/*     set position x_1 */
    }
    (*force)(btab, nb, *tptr + dt2);		/* get accel a_1 */
    dts4 = dt2 * dt2;
    for (p = btab, i = 0; p < btab+nb; p++) {	/* loop over bodies */
        pptr = Pos(p);				/*   get body coords */
	aptr = Acc(p);
	for (k = 0; k < NDIM; k++, i++) {	/*   loop over coords */
	    *pptr++ += dts4 * abak0[i];		/*     set position x_2 */
	    atmp1[i] = *aptr++;			/*     save accel a_1 */
	}
    }
    (*force)(btab, nb, *tptr + dt2);		/* get accel a_2 */
    for (p = btab, i = 0; p < btab+nb; p++) {	/* loop over bodies */
        pptr = Pos(p);				/*   get body coords */
	vptr = Vel(p);
	aptr = Acc(p);
	for (k = 0; k < NDIM; k++, i++) {	/*   loop over coords */
	    *pptr++ += dt2 * (*vptr++) + dts4 * (2*atmp1[i] - abak0[i]);
	    					/*     set position x_3 */
	    atmp2[i] = *aptr++;			/*     save accel a_2 */
	}
    }
    (*force)(btab, nb, *tptr + dt);		/* get accel a_3 */
    dt6 = dt / 6;
    dts6 = dt * dt6;
    for (p = btab, i = 0; p < btab+nb; p++) {	/* loop over bodies */
        pptr = Pos(p);				/*   get body coords */
	vptr = Vel(p);
	aptr = Acc(p);
	for (k = 0; k < NDIM; k++, i++) {	/*   loop over coords */
	    *vptr++ += dt6 * (abak0[i] + 2*atmp1[i] + 2*atmp2[i] + *aptr++);
	    *pptr++ += dts6 * (abak0[i] - 2*atmp1[i] + atmp2[i]);
	}
    }
    *tptr += dt;
}

/* 
 * PCSTEP: Predictor-Corrector step
 */
pcstep(btab, nb, tptr, force, dt)
bodyptr btab;		/* array of bodies */
int nb;			/* number of bodies */
real *tptr;		/* current time */
proc force;		/* acceleration calculation */
real dt;		/* integration time step */
{
    bodyptr p;
    int i, k;
    register real *pptr, *vptr, *aptr;
    real dt360, dts32, dt720, app, acp, acv;

    dt360 = dt / 360;
    for (p = btab, i = 0; p < btab+nb; p++) {	/* loop over bodies */
        pptr = Pos(p);				/*   get body coords */
	vptr = Vel(p);
	for (k = 0; k < NDIM; k++, i++) {	/*   loop over coords */
	    app = 323 * abak0[i] - 264 * abak1[i] +
		    159 * abak2[i] -  38 * abak3[i];
	    *pptr++ += dt * (*vptr++ + dt360 * app);
						/*     predict new position */
	}
    }
    (*force)(btab, nb, *tptr + dt);		/* find force at pred. pos. */
    dts32 = dt*dt / 32;
    dt720 = dt / 720;
    for (p = btab, i = 0; p < btab+nb; p++) {	/* loop over bodies */
        pptr = Pos(p);				/*   get body coords */
	vptr = Vel(p);
	aptr = Acc(p);
	for (k = 0; k < NDIM; k++, i++) {	/*   loop over coords */
	    acp = 3 * (*aptr) - 12 * abak0[i] +
		    18 * abak1[i] - 12 * abak2[i] + 3 * abak3[i];
	    *pptr++ += dts32 * acp;		/*     correct position */
	    acv = 251 * (*aptr++) + 646 * abak0[i] -
		    264 * abak1[i] + 106 * abak2[i] - 19 * abak3[i];
	    *vptr++ += dt720 * acv;		/*     advance velocity */
	}
    }
    *tptr += dt;				/* advance time */
}

eulerstep(btab, nb, tptr, force, dt)
bodyptr btab;		/* array of bodies */
int nb;			/* number of bodies */
real *tptr;		/* current time */
proc force;		/* acceleration calculation */
real dt;		/* integration time step */
{
    bodyptr p;
    int k;
    register real *pptr, *vptr, *aptr;

    for (p=btab; p<btab+nb; p++) {
        pptr = Pos(p);    
        vptr = Vel(p);
        aptr = Acc(p);
        for (k=0; k<NDIM; k++) {
            *pptr++ += dt * (*vptr);
            *vptr++ += dt * (*aptr++);
        }
    }
    *tptr += dt;
}

leapfrogstep(btab, nb, tptr, force, dt)
bodyptr btab;		/* array of bodies */
int nb;			/* number of bodies */
real *tptr;		/* current time */
proc force;		/* acceleration calculation */
real dt;		/* integration time step */
{
    error("leapfrog stepping not implemented");
}


rk4step(btab, nb, tptr, force, dt)
bodyptr btab;		/* array of bodies */
int nb;			/* number of bodies */
real *tptr;		/* current time */
proc force;		/* acceleration calculation */
real dt;		/* integration time step */
{
    body tmp1;
    bodyptr p, pt=&tmp1;
    int k;
    register real *pptr, *vptr, *aptr;
    vector dpos1, dvel1, dpos2, dvel2;
    real dth, dt6;

    dth = 0.5*dt;
    dt6 = dt/6.0;

    (*force)(btab,nb,*tptr);		/* needed only when fake physics */
    
    for (p=btab; p<btab+nb; p++) {

        for (k=0; k<NDIM; k++) {
            Pos(pt)[k] = Pos(p)[k] + dth*Vel(p)[k];
            Vel(pt)[k] = Vel(p)[k] + dth*Acc(p)[k];
        }

        (*force)(pt,1,*tptr+dth);

        SETV(dpos1, Vel(pt));                           /* save 'dyt' */
        SETV(dvel1, Acc(pt));
        for (k=0; k<NDIM; k++) {
            Pos(pt)[k] = Pos(p)[k] + dth*Vel(pt)[k];
            Vel(pt)[k] = Vel(p)[k] + dth*Acc(pt)[k];
        }

        (*force)(pt,1,*tptr+dth);
        
        SETV(dpos2, Vel(pt));                           /* save 'dym' */
        SETV(dvel2, Acc(pt));
        for (k=0; k<NDIM; k++) {
            Pos(pt)[k] = Pos(p)[k] + dt*Vel(pt)[k];
            Vel(pt)[k] = Vel(p)[k] + dt*Acc(pt)[k];
        }

        ADDV(dpos2, dpos2, dpos1);
        ADDV(dvel2, dvel2, dvel1);

        (*force)(pt,1,*tptr+dt);
        
        for (k=0; k<NDIM; k++) {
            Pos(p)[k] += dt6*(Vel(p)[k]+Vel(pt)[k]+2*dpos2[k]);
            Vel(p)[k] += dt6*(Acc(p)[k]+Acc(pt)[k]+2*dvel2[k]);
        }
    }
    *tptr += dt;
}

epistep(btab, nb, tptr, force, dt)
bodyptr btab;		/* array of bodies */
int nb;			/* number of bodies */
real *tptr;		/* current time */
proc force;		/* acceleration calculation */
real dt;		/* integration time step */
{
  bodyptr p;
  body tmp1;

  /* we'll just do one entry here and die */

  warning("Epicycle stepping");
  
  for (p=btab; p<btab+nb; p++) {
    (*force)(btab,nb,*tptr);	
  }

  error("DONE testing");

}



/* 
 * MOVEACCEL: local utility to stack back old values of the
 *	      forces. Only used in RK, PC and PC1.
 */
local void moveaccel(btab, nb)
bodyptr btab;		/* array of bodies */
int nb;			/* number of bodies */
{
    bodyptr p;
    int i, k;
    register real *aptr;

    for (p = btab, i = 0; p < btab+nb; p++) {	/* loop over bodies */
        aptr = Acc(p);				/*   get body accel */
	for (k = 0; k < NDIM; k++, i++) {	/*   loop over coords */
	    abak3[i] = abak2[i];		/*     copy history back */
	    abak2[i] = abak1[i];
	    abak1[i] = abak0[i];
	    abak0[i] = *aptr++;			/*     copy latest accel */
	}
    }
}

