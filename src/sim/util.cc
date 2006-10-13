//=========================================================================
//  UTIL.CC - part of
//
//                  OMNeT++/OMNEST
//           Discrete System Simulation in C++
//
//   Utility functions
//
//   Author: Andras Varga
//
//=========================================================================

/*--------------------------------------------------------------*
  Copyright (C) 1992-2005 Andras Varga

  This file is distributed WITHOUT ANY WARRANTY. See the file
  `license' for details on this and other legal matters.
*--------------------------------------------------------------*/

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "util.h"
#include "cenvir.h"
#include "csimulation.h"
#include "globals.h"
#include "cexception.h"
#include "cfunction.h"
#include "cscheduler.h" // dummy()
#include "distrib.h" // dummy()


//==========================================================================
// functions to support expressions compiled by nedtool

double min(double a, double b)      {return a<b ? a : b;}
double max(double a, double b)      {return a<b ? b : a;}

Define_Function( min,  2 )
Define_Function( max,  2 )


//==========================================================================
//=== utility functions

char *simtimeToStr(simtime_t t, char *buf)
{
   // Print simulation time into a string.
   // The result has two fields:
   //   a)  A real number meaning the sim.time expressed in seconds.
   //       Always has (at least) 8 significant digits
   //   b)  Time expressed in units such as us,ns,ms,s,m,h,d.
   //       The numbers in this field are truncated.
   // Conversion method (%f or %e) and lengths were chosen to
   // minimize fluctuations in total string length.
   // Result is typically 19,20 or 22 chars long.

   static char buf2[48];
   char *b = buf ? buf : buf2;

   if (t==0.0)
       sprintf(b,"0.0000000  ( 0.00s)");
   // Note that in the following line, a small constant is added to t
   // in order to eliminate truncation errors (like: "1.0000000e-6 (0us)")

   // TBD change int(...) to int(...+0.5) and remove +=1e-16; test a lot
   // with different values, e.g. 0.1 increments!
   else if (t+=1e-16, t<1e-9)
       sprintf(b,"%8.8e ( <1ns)", t);
   else if (t<1e-6)
       sprintf(b,"%8.8e (%3dns)", t,int(t*1e9));
   else if (t<1e-3)
       sprintf(b,"%8.8e (%3dus)", t,int(t*1e6));
   else if (t<1.0)
       sprintf(b,"%.10f (%3dms)", t,int(t*1e3));
   else if (t<60.0)
       sprintf(b,"%8.8g  (%2d.%2.2ds)", t,int(t*100)/100,int(t*100)%100);
   else if (t<3600.0)
       sprintf(b,"%8.8g (%2dm %2ds)", t,int(t)/60, int(t)%60);
   else if (t<86400.0)
       sprintf(b,"%8.8g (%2dh %2dm)", t,int(long(t)/3600L), int((long(t)%3600L)/60L));
   else if (t<8640000.0)
       sprintf(b,"%8.8g (%2dd %2dh)", t,int(t/86400L), int((long(t)%86400L)/3600L));
   else
       sprintf(b,"%8.8e (%2gd)", t,floor(t/86400L));

   return b;
}

char *simtimeToStrShort(simtime_t t, char *buf)
{
   // Print simulation time into a string -- short version.
   static char buf2[48];
   char *b = buf ? buf : buf2;

   if (t==0.0)
       sprintf(b,"0s");
   // Note that in the following line, a small constant is added to t
   // in order to eliminate truncation errors (like: "1.0000000e-6 (0us)")

   // TBD change int(...) to int(...+0.5) and remove +=1e-16; test a lot
   // with different values, e.g. 0.1 increments!
   else if (t+=1e-16, t<1e-9)
       sprintf(b,"%.5gs", t);
   else if (t<1e-6)
       sprintf(b,"%.5gns", t*1e9);
   else if (t<1e-3)
       sprintf(b,"%.5gus", t*1e6);
   else if (t<1.0)
       sprintf(b,"%.5gms", t*1e3);
   else if (t<3600.0)
       sprintf(b,"%.5gs", t);
   else if (t<86400.0)
       sprintf(b,"%dh %dm", int(long(t)/3600L), int((long(t)%3600L)/60L));
   else if (t<864000000.0)
       sprintf(b,"%dd %dh", int(t/86400L), int((long(t)%86400L)/3600L));
   else
       sprintf(b,"%.5gs", t);

   return b;
}

// *
// * old version of simtimeToStr():
// *
// char *simtimeToStr(simtime_t t, char *dest)
// {                                // place result either into dest
//       static char buf[32];       //   or into a static buffer
//       if (dest==NULL) dest=buf;
//
//       if (t<600)  // < 10 minutes: 's ms us ns' format
//       {
//           double tt = (double)t + 0.5e-9; //rounding
//           int   s = (int) tt;   tt -= s;   tt *= 1000.0;
//           int  ms = (int) tt;   tt -= ms;  tt *= 1000.0;
//           int  us = (int) tt;   tt -= us;  tt *= 1000.0;
//           int  ns = (int) tt;
//           sprintf(dest,"%3ds %3dms %3dus %3dns", s,ms,us,ns); // 22 chars
//       }
//       else if (t<48L*3600L) // < 3 days: 'h m s ms' format
//       {
//           double tt = ((double)t+0.5e-3)/3600.0; //hours + rounding
//           int   h = (int) tt;   tt -= h;   tt *= 60.0;
//           int   m = (int) tt;   tt -= m;   tt *= 60.0;
//           int   s = (int) tt;   tt -= s;   tt *= 1000.0;
//           int  ms = (int) tt;
//           sprintf(dest,"%2dh %2dm %3ds %3dms", h,m,s,ms); // 18 chars
//       }
//       else // >= 3 days: 'd h m s' format
//       {
//           double tt = ((double)t+0.005)/3600.0/24.0; //days + rounding
//           long  d = (int) tt;   tt -= d;   tt *= 24.0;
//           int   h = (int) tt;   tt -= h;   tt *= 60.0;
//           int   m = (int) tt;   tt -= m;   tt *= 60.0;
//           int   s = (int) tt;   tt -= s;   tt *= 100.0;
//           int   s100 = (int) tt;
//           sprintf(dest,"%ldd %2dh %2dm %3d.%.2d", d,h,m,s,s100); // 18+ chars
//       }
//
//       return dest;
// }

simtime_t strToSimtime(const char *str)
{
    while (*str==' ' || *str=='\t') str++;
    if (*str=='\0') return -1; // empty string not accepted
    simtime_t d = strToSimtime0( str );
    return (*str=='\0') ? d : -1; // OK if whole string could be interpreted
}

simtime_t strToSimtime0(const char *&str)
{
    double simtime = 0;

    while (*str!='\0')
    {
          // read number into num and skip it
          double num;
          int len;
          while (*str==' ' || *str=='\t') str++;
          if (0==sscanf(str, "%lf%n", &num, &len)) break; // break if error
          str+=len;

          // process time unit: d,h,m,s,ms,us,ns
          while (*str==' ' || *str=='\t') str++;
          if (str[0]=='n' && str[1]=='s')
                          {simtime += 1e-9*num; str+=2;}
          else if (str[0]=='u' && str[1]=='s')
                          {simtime += 1e-6*num; str+=2;}
          else if (str[0]=='m' && str[1]=='s')
                          {simtime += 1e-3*num; str+=2;}
          else if (str[0]=='s')
                          {simtime += num; str++;}
          else if (str[0]=='m')
                          {simtime += 60*num; str++;}
          else if (str[0]=='h')
                          {simtime += 3600*num; str++;}
          else if (str[0]=='d')
                          {simtime += 24*3600*num; str++;}
          else
                          {simtime += num; break;} // nothing can come after 'pure' number
    }
    return (simtime_t)simtime;
}

char *opp_strdup(const char *s)
{
    if (!s || !s[0]) return NULL;
    char *p = new char[ strlen(s)+1 ];
    if (p) strcpy(p,s);
    return p;
}

char *opp_strcpy(char *s1, const char *s2)
{
    if (s1 && s2)       return strcpy(s1,s2);
    else if (!s1 && s2) return NULL;
    else {s1[0]='\x0';  return s1;}
}

int opp_strcmp(const char *s1, const char *s2)
{
    // case-sensitive comparison
    if (s1 && s2)       return strcmp(s1,s2);
    else if (!s1 && s2) return -1;
    else if (s1 && !s2) return 1;
    else                return 0;
}

bool opp_strmatch(const char *s1, const char *s2)
{
    // case-sensitive comparison until
    // the length of the shorter string
    if (!s1 || !s2) {
         return true;
    } else {
         int l = Min(strlen(s1), strlen(s2));
         return strncmp( s1, s1, l)==0;
    }
}

int opp_strlen(const char *s)
{
    if (!s) return 0;
    return strlen(s);
}

char *opp_concat(const char *s1,
                 const char *s2,
                 const char *s3,
                 const char *s4)
{
    // concatenate strings into static buffer
    static char buf[256];
    char *dest=buf;
    if (s1) while( *s1 && dest!=buf+255 ) *dest++ = *s1++;
    if (s2) while( *s2 && dest!=buf+255 ) *dest++ = *s2++;
    if (s3) while( *s3 && dest!=buf+255 ) *dest++ = *s3++;
    if (s4) while( *s4 && dest!=buf+255 ) *dest++ = *s4++;
    *dest = 0;
    return buf;
}

char *opp_strprettytrunc(char *dest, const char *src, unsigned maxlen)
{
    if (!src) {
        *dest='\0';
        return dest;
    }
    strncpy(dest, src, maxlen);
    if (strlen(src)>maxlen) {
        dest[maxlen] = '\0';
        if (maxlen>=3) dest[maxlen-1] = dest[maxlen-2] = dest[maxlen-3] = '.';
    }
    return dest;
}

char *opp_mkindexedname(char *dest, const char *name, int index)
{
    static char buf[64];
    if (dest==NULL) dest=buf;
    sprintf(dest,"%.54s[%d]",name,index);
    return dest;
}

int opp_vsscanf(const char *s, const char *fmt, va_list va)
{
    // A simplified vsscanf implementation, solely for cStatistic::freadvarsf.
    // Only recognizes %d, %u, %ld, %g and whitespace. '#' terminates scanning

    int k=0;
    while (1)
    {
        if (*fmt=='%')
        {
            int n;
            if (fmt[1]=='d')
            {
                k+=sscanf(s,"%d%n",va_arg(va,int*),&n);
                s+=n; fmt+=2;
            }
            else if (fmt[1]=='u')
            {
                k+=sscanf(s,"%u%n",va_arg(va,unsigned int*),&n);
                s+=n; fmt+=2;
            }
            else if (fmt[1]=='l' && fmt[2]=='d')
            {
                k+=sscanf(s,"%ld%n",va_arg(va,long*),&n);
                s+=n; fmt+=3;
            }
            else if (fmt[1]=='l' && fmt[2]=='u')
            {
                k+=sscanf(s,"%lu%n",va_arg(va,unsigned long*),&n);
                s+=n; fmt+=3;
            }
            else if (fmt[1]=='l' && fmt[2]=='g')
            {
                k+=sscanf(s,"%lg%n",va_arg(va,double*),&n);
                s+=n; fmt+=3;
            }
            else if (fmt[1]=='g')
            {
                k+=sscanf(s,"%lg%n",va_arg(va,double*),&n);
                s+=n; fmt+=2;
            }
            else
            {
                throw new cRuntimeError("opp_vsscanf: unsupported format '%s'",fmt);
            }
        }
        else if (isspace(*fmt))
        {
            while (isspace(*s)) s++;
            fmt++;
        }
        else if (*fmt=='\0' || *fmt=='#')
        {
            return k;
        }
        else
        {
            throw new cRuntimeError("opp_vsscanf: unexpected char in format: '%s'",fmt);
        }
    }
}

//==========================================================================

void opp_error(ErrorCode errorcode...)
{
    va_list va;
    va_start(va, errorcode);
    char message[512];
    vsprintf(message, cErrorMessages::get(errorcode), va);
    va_end(va);

    throw new cRuntimeError(errorcode, message);
}

void opp_error(const char *msgformat...)
{
    va_list va;
    va_start(va, msgformat);
    char message[512];
    vsprintf(message, msgformat, va);
    va_end(va);

    throw new cRuntimeError(eUSER, message);
}

void opp_warning(ErrorCode errorcode...)
{
    va_list va;
    va_start(va, errorcode);
    char message[512];
    vsprintf(message, cErrorMessages::get(errorcode),va);
    va_end(va);

    if (!simulation.contextModule())
    {
        // we're called from global context
        ev.printfmsg("%s.", message);
    }
    else
    {
        ev.printfmsg("Module %s: %s.", simulation.contextModule()->fullPath().c_str(), message);
    }
}

void opp_warning(const char *msgformat...)
{
    va_list va;
    va_start(va, msgformat);
    char message[512];
    vsprintf(message,msgformat,va);
    va_end(va);

    if (!simulation.contextModule())
    {
        // we're called from global context
        ev.printfmsg("%s.", message);
    }
    else
    {
        ev.printfmsg("Module %s: %s.", simulation.contextModule()->fullPath().c_str(), message);
    }
}

void opp_terminate(ErrorCode errorcode...)
{
    va_list va;
    va_start(va, errorcode);
    char message[512];
    vsprintf(message, cErrorMessages::get(errorcode),va);
    va_end(va);

    throw new cTerminationException(errorcode,message);
}

void opp_terminate(const char *msgformat...)
{
    va_list va;
    va_start(va, msgformat);
    char message[512];
    vsprintf(message,msgformat,va);
    va_end(va);

    throw new cTerminationException(message);
}

//==========================================================================

const char *opp_typename(const std::type_info& t)
{
    if (t == typeid(std::string))
        return "std::string"; // otherwise we'd get "std::basic_string<........>"

    const char *s = t.name();

    // correct gcc 2.9x bug: it prepends the type name with its length
    while (*s>='0' && *s<='9') s++;

    // and MSVC prepends "class "...
    if (s[0]=='c' && s[1]=='l' && s[2]=='a' && s[3]=='s' && s[4]=='s' && s[5]==' ') s+=6;
    return s;
}

//==========================================================================

char *opp_quotestr(const char *txt)
{
    char *buf = new char[2*strlen(txt)+3]; // conservative guess
    char *d = buf;
    *d++ = '"';
    const char *s = txt;
    while (*s)
    {
        switch (*s)
        {
            case '\n': *d++ = '\\'; *d++ = 'n'; break;
            case '\r': *d++ = '\\'; *d++ = 'r'; break;
            case '\t': *d++ = '\\'; *d++ = 't'; break;
            case '"': *d++ = '\\'; *d++ = '"'; break;
            case '\\': *d++ = '\\'; *d++ = '\\'; break;
            default: *d++ = *s++;
        }
    }
    *d++ = '"';
    *d = '\0';
    return buf;
}

char *opp_parsequotedstr(const char *txt, const char *&endp)
{
    const char *s = txt;
    while (*s==' ' || *s=='\t')
        s++;
    if (*s++!='"')
        return NULL;  // no opening quote
    char *buf = new char [strlen(txt)+1];
    char *d = buf;
    while (*s && *s!='"')
    {
        if (*s++!='\\')
            *d++ = *--s;
        else if (*s=='n')
            *d++ = '\n';
        else if (*s=='r')
            *d++ = '\r';
        else if (*s=='t')
            *d++ = '\t';
        else
            *d++ = *s;
        s++;
    }
    *d = '\0';
    if (*s++!='"')
        {delete [] buf;return NULL;}  // no closing quote
    while (*s==' ' || *s=='\t')
        s++;
    endp = s;  // if (*s!='\0'), something comes after the string
    return buf;
}

//==========================================================================

cContextSwitcher::cContextSwitcher(cComponent *thisptr)
{
    // save current context and switch to new
    callerContext = simulation.context();
    simulation.setContext(thisptr);
}

cContextSwitcher::~cContextSwitcher()
{
    // restore old context
    if (!callerContext)
        simulation.setGlobalContext();
    else
        simulation.setContext(callerContext);
}

void cContextSwitcher::methodCall(const char *fmt,...)
{
    cComponent *methodContext = simulation.context();
    if (methodContext==callerContext)
        return;

    static char buf[MAX_METHODCALL];
    va_list va;
    va_start(va, fmt);
    vsprintf(buf,fmt,va);
    va_end(va);

    ev.methodCalled(callerContext, methodContext, buf);
}

//----

cContextTypeSwitcher::cContextTypeSwitcher(int ctxtype)
{
    // save current context type and switch to new one
    contexttype = simulation.contextType();
    simulation.setContextType(ctxtype);
}

cContextTypeSwitcher::~cContextTypeSwitcher()
{
    simulation.setContextType(contexttype);
}

//==========================================================================
// dummy function to force over-optimizing Unix linkers to include these symbols
// in the library

#include "cwatch.h"
#include "cstlwatch.h"
#include "cpacket.h"
#include "clcg32.h"
#include "cmersennetwister.h"
#include "cksplit.h"
#include "cpsquare.h"
#include "cstrtokenizer.h"
//void _dummy_for_env();
void _sim_dummy_func()
{
      cPacket x;
      cPacket y(x);
      x.info();
      cWatch_bool w(NULL,*(bool*)&x);
      std::vector<int> v;
      WATCH_VECTOR(v);
      w.supportsAssignment();
      exponential(1.0);
      cSequentialScheduler sch;
      sch.setSimulation(NULL);
      cLCG32 lcg;
      lcg.intRand();
      cMersenneTwister mt;
      mt.intRand();
      cKSplit ks;
      ks.info();
      cPSquare ps;
      ps.info();
      cStringTokenizer tok("");
      tok.nextToken();
      //_dummy_for_env();
}

