/* *************************************************************
****************************************************************
TSOIL45_lulc.CPP - object describing general characteristics of
                soil

****************************************************************
************************************************************* */

#include<cstdio>

  using std::printf;

#include<iostream>

  using std::cout;
  using std::ios;
  using std::cerr;
  using std::endl;

#include<fstream>

  using std::ifstream;
  using std::ofstream;

#include<cstdlib>

  using std::exit;
  using std::atof;
  using std:: atoi;

#include<cmath>

  using std::exp;
  using std::pow;
  using std::log;

#include<string>

  using std::string;


#include "tsoil45_lulc.h"

/* *************************************************************
************************************************************* */

Tsoil45::Tsoil45( void ) : ProcessXML45()
{

  text  = -99;
  wsoil = -99;

  pctsand = MISSING;
  pctsilt = MISSING;
  pctclay = MISSING;
  psiplusc = MISSING;


  awcapmm = MISSING;
  fldcap = MISSING;
  wiltpt = MISSING;
  totpor = MISSING;

  snowpack = MISSING;

  avlh2o = MISSING;
  moist = MISSING;
  pcfc = MISSING;
  pctp = MISSING;
  vsm = MISSING;

  rgrndh2o = MISSING;
  sgrndh2o = MISSING;

  snowinf = MISSING;
  rperc = MISSING;
  sperc = MISSING;
  rrun = MISSING;
  srun = MISSING;
  h2oyld = MISSING;

  org.carbon = MISSING;     // need different pools or not? MJ MLS
  org.nitrogen = MISSING;   // need different pools or not? MJ MLS

  availn = MISSING;         // need different pools or not? MJ MLS

  yrsnowpack = MISSING;

  yravlh2o = MISSING;
  yrsmoist = MISSING;
  yrpctp = MISSING;
  yrvsm = MISSING;
  meanvsm = MISSING;

  yrrgrndh2o = MISSING;
  yrsgrndh2o = MISSING;

  yrsnowinf = MISSING;
  yrrperc = MISSING;
  yrsperc = MISSING;
  yrrrun = MISSING;
  yrsrun = MISSING;
  yrh2oyld = MISSING;

  yrorgc = MISSING;
  yrorgc_active = MISSING;
  yrorgc_slow = MISSING;
  yrorgc_passive = MISSING;

  yrorgn = MISSING;
  yrorgn_active = MISSING;
  yrorgn_slow = MISSING;
  yrorgn_passive = MISSING;

  yrc2n_active = MISSING;
  yrc2n_slow = MISSING;
  yrc2n_passive = MISSING;

  yravln = MISSING;   // need different pools or not? MJ MLS

  ninput = MISSING;
  yrnin = MISSING;
  yrnin_active = MISSING;
  yrnin_slow = MISSING;
  yrnin_passive = MISSING;


  nlost = MISSING;
  yrnlost = MISSING;
  yrnlost_active = MISSING;
  yrnlost_slow = MISSING;
  yrnlost_passive = MISSING;

  yrlchdin = MISSING;

  // Number of days per month

  ndays[0] = 31;
  ndays[1] = 28;
  ndays[2] = 31;
  ndays[3] = 30;
  ndays[4] = 31;
  ndays[5] = 30;
  ndays[6] = 31;
  ndays[7] = 31;
  ndays[8] = 30;
  ndays[9] = 31;
  ndays[10] = 30;
  ndays[11] = 31;

};

/* *************************************************************
************************************************************* */



/* *************************************************************
************************************************************* */
// Getting rooting depth, important for MLS
// currently non-dynamic, check if there's way to make rooting depth dynamic?
void Tsoil45::getrootz( const string& ecd )
{
  ifstream infile;
  int dcmnt;
  int comtype;

  infile.open( ecd.c_str(), ios::in );

  if( !infile )
  {
    cerr << endl;
    cerr << "Cannot open " << ecd << " for root ECD input";
    cerr << endl;
    exit( -1 );
  }

  getXMLrootNode( infile, "rootzECD" );

  for( dcmnt = 1; dcmnt < MAXCMNT; ++dcmnt )
  {

    comtype = getXMLcommunityNode( infile, "rootzECD" );

    if( comtype >= MAXCMNT )
    {
      cerr << endl << "comtype is >= MAXCMNT" << endl;
      cerr << "comtype cannot be greater than " << (MAXCMNT-1);
      cerr << " in leafECD" << endl;
      exit( -1 );
    }

    rootzc[comtype] = getXMLcmntArrayDouble( infile,
                                             "rootzECD",
                                             "rootzc",
                                             comtype );

    minrootz[comtype] = getXMLcmntArrayDouble( infile,
                                               "rootzECD",
                                               "minrootz",
                                               comtype );

/* added coefa, coefb, frrootc_fb, frrootc_am and frrootc_mn MJ MLS; 
 *check for feasibility within this function;
 */ 

/*    coefa[comtype] = getXMLcmntArrayDouble( infile,
                                               "rootzECD",
                                               "coefa",
                                               comtype );

    coefb[comtype] = getXMLcmntArrayDouble( infile,
                                               "rootzECD",
                                               "coefb",
                                               comtype );


    frrootc_fb[comtype] = getXMLcmntArrayDouble( infile,
                                               "rootzECD",
                                               "frrootc_fb",
                                               comtype );


    frrootc_am[comtype] = getXMLcmntArrayDouble( infile,
                                               "rootzECD",
                                               "frrootc_am",
                                               comtype );


    frrootc_mn[comtype] = getXMLcmntArrayDouble( infile,
                                               "rootzECD",
                                               "frrootc_mn",
                                               comtype );
*/
     endXMLcommunityNode( infile );
  }

  if ( dcmnt < MAXCMNT )
  {
    cerr << endl << " Parameters found for only " << dcmnt;
    cerr << " community types out of a maximum of ";
    cerr << (MAXCMNT-1) << " types in rootzECD" << endl;
    exit( -1 );
  }

  infile.close();

};

/* *************************************************************
************************************************************* */

void Tsoil45::lake( const double& tair,
                    const double& prec,
                    double& rain,
                    double& snowfall,
       		          const double& pet,
                    double& eet )
{

  rgrndh2o = ZERO;
  sperc = ZERO;
  snowpack = ZERO;
  sgrndh2o = ZERO;
  moist = ZERO;

  if ( tair >= -1.0 )
  {
   rain = prec;
    snowfall = ZERO;
  }
  else
  {
    rain = ZERO;
    snowfall = prec;
  }

  eet = pet;
  h2oyld = prec - pet;

};

/* *************************************************************
************************************************************* */


/* *************************************************************
************************************************************* */

void Tsoil45::percol( const double& rain)
{

  double extra;
  double recharge;
  sperc = ZERO;
  rperc = ZERO;

  if( avlh2o > awcapmm )
  {
    extra = 10.0*(avlh2o - awcapmm);
    recharge = rain + snowinf;
    if( recharge <= ZERO ) { recharge = 0.001; }
    sperc = extra*snowinf/recharge;
    rperc = extra*rain/recharge;
    // moisture loss above field capacity with a characteristic time of a 3 days
  }

};

/* *************************************************************
************************************************************* */


/* *************************************************************
************************************************************* */

void Tsoil45::resetMonthlyFluxes( void )
{
  // Reset monthly fluxes to zero

  // Nitrogen fluxes

  ninput = ZERO;
  nlost = ZERO;
  nlost_active = ZERO;
  nlost_slow = ZERO;
  nlost_passive = ZERO;


  // Water fluxes

// Comment out next two lines in MITTEM as these values come
//   from CLM rather than TEM

  ineet = ZERO;
  eet = ZERO;

  rperc = ZERO;
  sperc = ZERO;
  rrun = ZERO;
  srun = ZERO;

  snowinf = ZERO;
  h2oyld = ZERO;


};

/* *************************************************************
************************************************************* */


/* *************************************************************
************************************************************* */

void Tsoil45::resetYrFluxes( void )
{
  // Reset annual fluxes and summary variables to zero

  // Annual carbon storage

  yrorgc = ZERO;
  yrorgc_active = ZERO;
  yrorgc_slow = ZERO;
  yrorgc_passive = ZERO;

  // Annual nitrogen storage

  yrorgn = ZERO;
  yrorgn_active = ZERO;
  yrorgn_slow = ZERO;
  yrorgn_passive = ZERO;

  yravln = ZERO;

  // Annual water storage

  yravlh2o = ZERO;
  yrsmoist = ZERO;
  yrvsm = ZERO;
  yrpctp = ZERO;
  yrsnowpack = ZERO;
  yrrgrndh2o = ZERO;
  yrsgrndh2o = ZERO;


  // Annual nitrogen fluxes

  yrnin = ZERO;
  yrnin_active = ZERO;
  yrnin_slow = ZERO;
  yrnin_passive = ZERO;

  yrnlost = ZERO;
  yrnlost_active = ZERO;
  yrnlost_slow = ZERO;
  yrnlost_passive = ZERO;

  yrlchdin = ZERO;

  // Annual water fluxes

  yrineet = ZERO;
  yreet = ZERO;
  yrrperc = ZERO;
  yrsperc = ZERO;
  yrrrun = ZERO;
  yrsrun = ZERO;

  yrsnowinf = ZERO;
  yrh2oyld = ZERO;

};

/* *************************************************************
************************************************************* */


/* *************************************************************
************************************************************* */

double Tsoil45::rrunoff( const double& rgrndh2o )
{

  double rrunof;

  rrunof = 0.5 * (rgrndh2o + rperc);

  return rrunof;

};

/* *************************************************************
************************************************************* */

/* *************************************************************
************************************************************* */

void Tsoil45::setSWP( void )
{
  double pota, potb;
  
  pota = 100.0*exp(-4.396-0.0715*pctclay-0.000488*pctsand*pctsand-
		0.00004285*pctsand*pctsand*pctclay);
  
  potb = -3.14 -0.00222*pctclay*pctclay -
		0.00003484*pctsand*pctsand*pctclay;
		
  if( vsm > 0.01) { swp = -0.001 * pota * pow(vsm,potb);}
  else { swp = -0.001 * pota * pow( 0.01 , potb);}
  
  gm = (1.0 + exp(0.5*(-1.5)))/(1.0 + exp(0.5*(-1.5 - swp)));
  
};

/* *************************************************************
************************************************************* */


/* *************************************************************
************************************************************* */

void Tsoil45::setKH2O( const double& vsm,
                       const int& moistlim )
{
  double vfc;

  if( 0 == moistlim )
  {
    vfc = pcfldcap * 0.01;

    kh2o = pow( vfc, 3.0 );
//    kh2o = 1.0;
  }
  else
  {
    if( vsm > 1.0 ) { kh2o = 1.0; }
    else { kh2o = pow( vsm, 3.0 ); }
//    kh2o = 1.0 + 0.5*exp( 20.0*((pcwiltpt+pcfldcap)/200.0-vsm));
//    kh2o = 1.0/kh2o;
  }


};

/* *************************************************************
************************************************************* */

/* *************************************************************
************************************************************* */

void Tsoil45::showecd( void )
{

  cout << endl << "                   SOIL CHARACTERISTICS OF SITE";
  cout << endl << endl;
  printf( "PSAND    = %5.2lf      PSILT = %5.2lf      PCLAY = %5.2lf\n",
          pctsand,
          pctsilt,
          pctclay );

  printf( "POROSITY = %5.2lf   PCFLDCAP = %5.2lf   PCWILTPT = %5.2lf\n",
          pctpor,
          pcfldcap,
          pcwiltpt );

};

/* *************************************************************
************************************************************* */


/* *************************************************************
************************************************************* */

double Tsoil45::snowmelt( const double& elev,
                          const double& tair,
                          const double& prevtair,
                          const double& psnowpack )
{

  double snowflux = ZERO;

  if( tair >= -1.0 )
  {
    if( elev <= 500.0 ) { snowflux = psnowpack;}
    else
    {
      if( prevtair < -1.0 ) { snowflux = 0.5 * psnowpack; }
      else { snowflux = psnowpack; }
    }
  }

  if( snowflux < ZERO ) { snowflux = ZERO; }

  return snowflux;

};

/* *************************************************************
************************************************************* */


/* *************************************************************
************************************************************* */

double Tsoil45::srunoff( const double& elev,
                         const double& tair,
                         const double& prevtair,
                         const double& prev2tair,
                         const double& sgrndh2o )
{

  double srunof = ZERO;

  if( tair >= -1.0 )
  {
    if( prevtair < -1.0 )
    {
      srunof = 0.1 * (sgrndh2o + sperc);
    }
    else
    {
      if( prev2tair < -1.0 )
      {
	    if( elev <= 500.0 )
        {
          srunof = 0.5 * (sgrndh2o + sperc);
        }
	    else { srunof = 0.25 * (sgrndh2o + sperc); }
      }
      else { srunof = 0.5 * (sgrndh2o + sperc); }
    }
  }

  return srunof;

};


/* *************************************************************
************************************************************* */

void Tsoil45::updateHydrology( const double& elev,
                               const double& tair,
                               const double& prevtair,
                               const double& prev2tair,
                               const double& rain,
                               const double& pet,
                               const double& avlh2o,
                               const double& rgrndh2o,
                               const double& sgrndh2o,
                               const int& irrgflag,
                               double& irrigate,
                               const int& pdm )
{
  // veg.pet is actually eet, so just set ineet and eet to pet

  ineet = pet;
  eet = ineet;

  // Determine percolation of rain water (rperc) and snow melt
  //   water (sperc) through the soil profile

  percol( rain );

  // Determine runoff derived from rain (soil.rrun) and/or
  //   snow (soil.srun)

  rrun = rrunoff( rgrndh2o );

  srun = srunoff( elev,
                  tair,
                  prevtair,
                  prev2tair,
                  sgrndh2o );

};


/* *************************************************************
************************************************************* */
void Tsoil45::updateDOCLEACH( const double& doc,
                              const double& sh2o )
//rrun and srun need update to multiple horizons MJ MLS;
{

   lchdoc = (doc/(sh2o+rrun+srun))*(rrun+srun);

};


/* *************************************************************
************************************************************* */

void Tsoil45::updateNLosses( const int& pdcmnt,
                             const double& h2oloss,
                           const double& availn,
                           const double& soilh2o )
{
   nlost = (availn/(soilh2o+rrun+srun))*(rrun+srun);
   
//cout << "nlostmn1 = " << nlostmn << " " << availnmn << endl;
/*
  if( soilh2o > ZERO )
 { 
    nlost = availn / (soilh2o+h2oloss);

    nlost *= (h2oloss + (rootz * 1000.0))
             / (rootz * 1000.0);

    nlost *= nloss[pdcmnt];
  } 
  else { nlost = ZERO; } */ 
  //nlost *= nloss[pdcmnt];           
  nlost_active *= nloss_active[pdcmnt]; 
  nlost_slow *= nloss_slow[pdcmnt];
  nlost_passive *= nloss_passive[pdcmnt];

//  nlost = nlost_active + nlost_slow + nlost_passive;

};

/* *************************************************************
************************************************************* */


/* *************************************************************
************************************************************* */

double Tsoil45::updateRootZ( const int& pdcmnt,
                             const double& sh2o,
                             const double& finerootc )

{

  double pota, potb;
  
  //Saxton Equations 
  pota = 100.0*exp(-4.396-0.0715*pctclay-0.000488*pctsand*pctsand-
		0.00004285*pctsand*pctsand*pctclay);
  
  potb = -3.14 -0.00222*pctclay*pctclay -
		0.00003484*pctsand*pctsand*pctclay;

  rootz = rootzc[pdcmnt];
  if( rootz < minrootz[pdcmnt] ) { rootz = minrootz[pdcmnt]; }

  pctpor = 100.0*(0.332 - 0.0007251*pctsand + 0.1276*log10(pctclay));
  pcfldcap = 100.0*pow(33.0/pota, 1.0/potb);
  pcwiltpt = 100.0*pow(1500.0/pota, 1.0/potb);
  //pcwiltpt = 100.0*pow(10000.0/pota, 1.0/potb);
// Inverting the saxton equation psi = -0.001*pota*vsm^potb
// And using the definition of fldcap = -0.033 MPa; wiltpt = -1.5 MPa

  totpor  = rootz * pctpor * 10.0;
  fldcap  = rootz * pcfldcap * 10.0;
  wiltpt  = rootz * pcwiltpt * 10.0;

  awcapmm = fldcap - wiltpt;

  return (sh2o / (rootz * 1000.0));

};

/* *************************************************************
************************************************************* */


/* *************************************************************
************************************************************* */

void Tsoil45::xtext( const int& pdcmnt,
                     const double& pctsilt,
                     const double& pctclay )
{

  double pota, potb;
  
  totpor = fldcap = wiltpt = MISSING;
  awcapmm =  MISSING;

  psiplusc = (pctsilt + pctclay) * 0.01;
  if( psiplusc < 0.01 ) { psiplusc = 0.01; }
  
  //Saxton Equations  
  pota = 100.0*exp(-4.396-0.0715*pctclay-0.000488*pctsand*pctsand-
		0.00004285*pctsand*pctsand*pctclay);
  
  potb = -3.14 -0.00222*pctclay*pctclay -
		0.00003484*pctsand*pctsand*pctclay;

  rootz = rootzc[pdcmnt];
  if( rootz < minrootz[pdcmnt] ) { rootz = minrootz[pdcmnt]; }

  pctpor = 100.0*(0.332 - 0.0007251*pctsand + 0.1276*log10(pctclay));
  pcfldcap = 100.0*pow(33.0/pota, 1.0/potb);
  pcwiltpt = 100.0*pow(1500.0/pota, 1.0/potb);
// Inverting the saxton equation psi = -0.001*pota*vsm^potb
// And using the definition of fldcap = -0.033 MPa; wiltpt = -1.5 MPa

  totpor  = rootz * pctpor * 10.0;
  fldcap  = rootz * pcfldcap * 10.0;
  wiltpt  = rootz * pcwiltpt * 10.0;

  awcapmm = fldcap - wiltpt;

};

