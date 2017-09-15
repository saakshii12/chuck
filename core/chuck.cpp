/*----------------------------------------------------------------------------
 ChucK Concurrent, On-the-fly Audio Programming Language
   Compiler and Virtual Machine
 
 Copyright (c) 2003 Ge Wang and Perry R. Cook.  All rights reserved.
   http://chuck.stanford.edu/
   http://chuck.cs.princeton.edu/
 
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 U.S.A.
 -----------------------------------------------------------------------------*/

//-----------------------------------------------------------------------------
// file: chuck.h
// desc: chuck engine header; VM + compiler + state; no audio I/O
//       REFACTOR-2017
//
// author: Ge Wang (http://www.gewang.com/)
// date: fall 2017
//
// additional authors:
//         Jack Atherton (lja@ccrma.stanford.edu)
//         Spencer Salazar (spencer@ccrma.stanford.edu)
//-----------------------------------------------------------------------------
#include "chuck.h"
#include "chuck_errmsg.h"
#include "chuck_otf.h"
#include "ulib_machine.h"
#include "util_string.h"




// chuck param defaults
#define CHUCK_PARAM_SAMPLE_RATE_DEFAULT          "44100"
#define CHUCK_PARAM_INPUT_CHANNELS_DEFAULT       "2"
#define CHUCK_PARAM_OUTPUT_CHANNELS_DEFAULT      "2"
#define CHUCK_PARAM_VM_ADAPTIVE_DEFAULT          "0"
#define CHUCK_PARAM_VM_HALT_DEFAULT              "0"
#define CHUCK_PARAM_OTF_PORT_DEFAULT             "8888"
#define CHUCK_PARAM_DUMP_INSTRUCTIONS_DEFAULT    "0"
#define CHUCK_PARAM_DEPRECATE_LEVEL_DEFAULT      "1"
#define CHUCK_PARAM_WORKING_DIRECTORY_DEFAULT    ""
#define CHUCK_PARAM_CHUGIN_DIRECTORY_DEFAULT     ""



// chuck statics
// version
const char ChucK::VERSION[] = "1.3.6.0-rc1 (numchuck)";
t_CKUINT ChucK::o_numVMs = 0;




//-----------------------------------------------------------------------------
// name: version()
// desc: get chuck version
//-----------------------------------------------------------------------------
const char * ChucK::version()
{
    return VERSION;
}




//-----------------------------------------------------------------------------
// name: intSize()
// desc: get chuck int size (in bits)
//-----------------------------------------------------------------------------
t_CKUINT ChucK::intSize()
{
    return machine_intsize();
}




//-----------------------------------------------------------------------------
// name: ChucK()
// desc: constructor
//-----------------------------------------------------------------------------
ChucK::ChucK()
{
    // instantiate the carrier!
    m_carrier = new Chuck_Carrier();
    // increment the numChucKs
    o_numVMs++;
    // initialize default params
    initDefaultParams();
    // did user init?
    m_init = FALSE;
}




//-----------------------------------------------------------------------------
// name: ~ChucK()
// desc: destructor
//-----------------------------------------------------------------------------
ChucK::~ChucK()
{
    // shutdown
    shutdown();
    // decrement the numChucKs
    o_numVMs--;
}




// REFACTOR-2017: TODO Ge: Implementation of this?
//-----------------------------------------------------------------------------
// name: ck_param_type
// desc: enum for type of param (int, float, string)
//-----------------------------------------------------------------------------
enum ck_param_type
{
    ck_param_int, ck_param_float, ck_param_string
};



//-----------------------------------------------------------------------------
// name: ck_param_types
// desc: storage for types of param (int, float, string)
//-----------------------------------------------------------------------------
std::map< std::string, ck_param_type> ck_param_types;




//-----------------------------------------------------------------------------
// name: initDefaultParams()
// desc: initialize default params
//-----------------------------------------------------------------------------
void ChucK::initDefaultParams()
{
    m_params[CHUCK_PARAM_SAMPLE_RATE] = CHUCK_PARAM_SAMPLE_RATE_DEFAULT;
    m_params[CHUCK_PARAM_INPUT_CHANNELS] = CHUCK_PARAM_INPUT_CHANNELS_DEFAULT;
    m_params[CHUCK_PARAM_OUTPUT_CHANNELS] = CHUCK_PARAM_OUTPUT_CHANNELS_DEFAULT;
    m_params[CHUCK_PARAM_VM_ADAPTIVE] = CHUCK_PARAM_VM_ADAPTIVE_DEFAULT;
    m_params[CHUCK_PARAM_VM_HALT] = CHUCK_PARAM_VM_HALT_DEFAULT;
    m_params[CHUCK_PARAM_OTF_PORT] = CHUCK_PARAM_OTF_PORT_DEFAULT;
    m_params[CHUCK_PARAM_DUMP_INSTRUCTIONS] = CHUCK_PARAM_DUMP_INSTRUCTIONS_DEFAULT;
    m_params[CHUCK_PARAM_DEPRECATE_LEVEL] = CHUCK_PARAM_DEPRECATE_LEVEL_DEFAULT;
    m_params[CHUCK_PARAM_WORKING_DIRECTORY] = CHUCK_PARAM_WORKING_DIRECTORY_DEFAULT;
    m_params[CHUCK_PARAM_CHUGIN_DIRECTORY] = CHUCK_PARAM_CHUGIN_DIRECTORY_DEFAULT;
    
    ck_param_types[CHUCK_PARAM_SAMPLE_RATE] =       ck_param_int;
    ck_param_types[CHUCK_PARAM_INPUT_CHANNELS] =    ck_param_int;
    ck_param_types[CHUCK_PARAM_OUTPUT_CHANNELS] =   ck_param_int;
    ck_param_types[CHUCK_PARAM_VM_ADAPTIVE] =       ck_param_int;
    ck_param_types[CHUCK_PARAM_VM_HALT] =           ck_param_int;
    ck_param_types[CHUCK_PARAM_OTF_PORT] =          ck_param_int;
    ck_param_types[CHUCK_PARAM_DUMP_INSTRUCTIONS] = ck_param_int;
    ck_param_types[CHUCK_PARAM_DEPRECATE_LEVEL] =   ck_param_int;
    ck_param_types[CHUCK_PARAM_WORKING_DIRECTORY] = ck_param_string;
    ck_param_types[CHUCK_PARAM_CHUGIN_DIRECTORY] =  ck_param_string;
}




//-----------------------------------------------------------------------------
// name: setParam()
// desc: set an int param by name
//-----------------------------------------------------------------------------
bool ChucK::setParam( const std::string & name, t_CKINT value )
{
    if( m_params.count( name ) > 0 && ck_param_types[name] == ck_param_int )
    {
        std::ostringstream s;
        s << value;
        m_params[name] = s.str();
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}




//-----------------------------------------------------------------------------
// name: setParam()
// desc: set a float param by name
//-----------------------------------------------------------------------------
bool ChucK::setParam( const std::string & name, t_CKFLOAT value )
{
    if( m_params.count( name ) > 0 && ck_param_types[name] == ck_param_float )
    {
        std::ostringstream s;
        s << value;
        m_params[name] = s.str();
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}




//-----------------------------------------------------------------------------
// name: setParam()
// desc: set a string param by name
//-----------------------------------------------------------------------------
bool ChucK::setParam( const std::string & name, const std::string & value )
{
    if( m_params.count( name ) > 0 && ck_param_types[name] == ck_param_string )
    {
        m_params[name] = value;
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}




//-----------------------------------------------------------------------------
// name: getParamInt()
// desc: get an int param
//-----------------------------------------------------------------------------
t_CKINT ChucK::getParamInt( const std::string & key )
{
    t_CKINT result = 0;
    if( m_params.count( key ) > 0 && ck_param_types[key] == ck_param_int )
    {
        istringstream s( m_params[key] );
        s >> result;
    }
    return result;
}




//-----------------------------------------------------------------------------
// name: getParamInt()
// desc: get a float param
//-----------------------------------------------------------------------------
t_CKFLOAT ChucK::getParamFloat( const std::string & key )
{
    t_CKFLOAT result = 0;
    if( m_params.count( key ) > 0 && ck_param_types[key] == ck_param_float )
    {
        istringstream s( m_params[key] );
        s >> result;
    }
    return result;
}




//-----------------------------------------------------------------------------
// name: getParamString()
// desc: get a string param
//-----------------------------------------------------------------------------
std::string ChucK::getParamString( const std::string & key )
{
    if( m_params.count( key ) > 0 && ck_param_types[key] == ck_param_int )
    {
        return m_params[key];
    }
    else
    {
        return "";
    }
}




//-----------------------------------------------------------------------------
// initialize ChucK (using params)
//-----------------------------------------------------------------------------
bool ChucK::init()
{
    // sanity check
    if( m_carrier->vm != NULL )
    {
        CK_FPRINTF_STDERR( "[chuck]: VM already initialized...\n" );
        return false;
    }
    
    // get VM params
    t_CKUINT srate = getParamInt( CHUCK_PARAM_SAMPLE_RATE );
    t_CKUINT outs = getParamInt( CHUCK_PARAM_OUTPUT_CHANNELS );
    t_CKUINT ins = getParamInt( CHUCK_PARAM_INPUT_CHANNELS );
    t_CKUINT adaptiveSize = getParamInt( CHUCK_PARAM_VM_ADAPTIVE );
    t_CKBOOL halt = getParamInt( CHUCK_PARAM_VM_HALT ) != 0;
    
    // get compiler params
    t_CKBOOL dump = getParamInt( CHUCK_PARAM_DUMP_INSTRUCTIONS ) != 0;
    t_CKBOOL auto_depend = getParamInt( CHUCK_PARAM_AUTO_DEPEND ) != 0;
    string workingDir = getParamString( CHUCK_PARAM_WORKING_DIRECTORY );
    string chuginDir = getParamString( CHUCK_PARAM_CHUGIN_DIRECTORY );
    t_CKUINT deprecate = getParamInt( CHUCK_PARAM_DEPRECATE_LEVEL );
    // TODO: chugin load option
    // list of search pathes (added 1.3.0.0)
    std::list<std::string> dl_search_path;
    if( workingDir != std::string("") )
    {
        dl_search_path.push_back( workingDir );
    }
    if( chuginDir != std::string("") )
    {
        dl_search_path.push_back( chuginDir );
    }
    // list of individually named chug-ins (added 1.3.0.0)
    std::list<std::string> named_dls;
    
    
    // instantiate VM
    m_carrier->vm = new Chuck_VM();
    // reference back to carrier
    m_carrier->vm->setCarrier( m_carrier );
    // initialize VM
    if( !m_carrier->vm->initialize( srate, outs, ins, adaptiveSize, halt ) )
    {
        CK_FPRINTF_STDERR( "[chuck]: %s\n", m_carrier->vm->last_error() );
        goto cleanup;
    }
    
    // instantiate compiler
    m_carrier->compiler = new Chuck_Compiler();
    // reference back to carrier
    m_carrier->compiler->setCarrier( m_carrier );
    
    // initialize compiler
    if( !m_carrier->compiler->initialize( dl_search_path, named_dls ) )
    {
        CK_FPRINTF_STDERR( "[chuck]: compiler failed to initialize...\n" );
        goto cleanup;
    }
    // set dump flag
    m_carrier->compiler->emitter->dump = dump;
    // set auto depend flag (for type checker) | currently must be FALSE
    m_carrier->compiler->set_auto_depend( auto_depend );
    // set deprecation level
    m_carrier->env->deprecate_level = deprecate;
    
    // VM + type system integration
    if( !m_carrier->vm->initialize_synthesis() )
    {
        CK_FPRINTF_STDERR( "[chuck]: %s\n", m_carrier->vm->last_error() );
        goto cleanup;
    }
    
    // TODO: load chugins
    
    // load user namespace
    m_carrier->env->load_user_namespace();
    
    // did user init?
    m_init = TRUE;
    
    return true;
    
cleanup:
    SAFE_RELEASE( m_carrier->chout );
    SAFE_RELEASE( m_carrier->cherr );
    SAFE_DELETE( m_carrier->vm );
    SAFE_DELETE( m_carrier->compiler );
    m_carrier->env = NULL;
    m_carrier->otf_socket = NULL; // TODO: properly dealloc
    m_carrier->otf_port = 0;
    m_carrier->otf_thread = 0; // TODO: properly dealloc
    
    return false;
}




//-----------------------------------------------------------------------------
// shutdown ChucK
//-----------------------------------------------------------------------------
bool ChucK::shutdown()
{
    // stop VM
    if( m_carrier != NULL && m_carrier->vm != NULL  )
    {
        m_carrier->vm->stop();

        // wait for it to stop...
        while( m_carrier->vm->running() )
        {
            usleep(1000);
        }
    }

    // free vm, compiler, friends
    SAFE_RELEASE( m_carrier->chout );
    SAFE_RELEASE( m_carrier->cherr );
    SAFE_DELETE( m_carrier->vm );
    SAFE_DELETE( m_carrier->compiler );
    m_carrier->otf_socket = NULL; // TODO: properly dealloc
    m_carrier->otf_port = 0;
    m_carrier->otf_thread = 0; // TODO: properly dealloc

    // done
    return true;
}




//-----------------------------------------------------------------------------
// name: compileFile()
// desc: compile a file (can be called anytime)
//-----------------------------------------------------------------------------
bool ChucK::compileFile( const std::string & path, const std::string & argsTogether, int count )
{
    // sanity check
    if( !m_carrier->compiler )
    {
        // error
        CK_FPRINTF_STDERR( "[chuck]: compileFile() invoked before initialization ...\n" );
        return false;
    }
    
    string filename;
    vector<string> args;
    Chuck_VM_Code * code = NULL;
    Chuck_VM_Shred * shred = NULL;
    
    // log
    EM_log( CK_LOG_FINE, "compiling '%s'...", filename.c_str() );
    // push indent
    EM_pushlog();
    
    // append
    string theThing = path + ":" + argsTogether;
    
    // parse out command line arguments
    if( !extract_args( theThing, filename, args ) )
    {
        // error
        CK_FPRINTF_STDERR( "[chuck]: malformed filename with argument list...\n" );
        CK_FPRINTF_STDERR( "    -->  '%s'", theThing.c_str() );
        return false;
    }
    
    // construct full path to be associated with the file so me.sourceDir() works
    // (added 1.3.0.0)
    std::string full_path = get_full_path(filename);
    
    // parse, type-check, and emit (full_path added 1.3.0.0)
    if( !m_carrier->compiler->go( filename, NULL, NULL, full_path ) )
        return false;
    
    // get the code
    code = m_carrier->compiler->output();
    // name it
    code->name += path;
    
    // log
    EM_log( CK_LOG_FINE, "sporking %d %s...", count,
           count == 1 ? "instance" : "instances" );
    
    // spork it
    while( count-- )
    {
        // spork (for now, spork_immediate arg is always false)
        shred = m_carrier->vm->spork( code, NULL, FALSE );
        // add args
        shred->args = args;
    }
    
    // pop indent
    EM_poplog();
    
    return true;
}




//-----------------------------------------------------------------------------
// name: compileCode()
// desc: compile code directly
//-----------------------------------------------------------------------------
bool ChucK::compileCode( const std::string & code, const std::string & argsTogether, int count)
{
    // sanity check
    if( !m_carrier->compiler )
    {
        // error
        CK_FPRINTF_STDERR( "[chuck]: compileCode() invoked before initialization ...\n" );
        return false;
    }
    
    vector<string> args;
    Chuck_VM_Code * vm_code = NULL;
    Chuck_VM_Shred * shred = NULL;
    
    // log
    EM_log( CK_LOG_FINE, "compiling string..." );
    // push indent
    EM_pushlog();
    
    // falsify filename / path for various logs
    string theThing = "compiled.code:" + argsTogether;
    string fakefakeFilename = "<result file name goes here>";
    
    // parse out command line arguments
    if( !extract_args( theThing, fakefakeFilename, args ) )
    {
        // error
        CK_FPRINTF_STDERR( "[chuck]: malformed filename with argument list...\n" );
        CK_FPRINTF_STDERR( "    -->  '%s'", theThing.c_str() );
        return false;
    }
    
    // PARAM
    std::string workingDir = getParamString( CHUCK_PARAM_WORKING_DIRECTORY );
    
    // construct full path to be associated with the file so me.sourceDir() works
    std::string full_path = workingDir + "/compiled.code";
    
    // parse, type-check, and emit (full_path added 1.3.0.0)
    if( !m_carrier->compiler->go( "<compiled.code>", NULL, code.c_str(), full_path ) )
        return false;
    
    // get the code
    vm_code = m_carrier->compiler->output();
    // name it (no path to append)
    vm_code->name += "compiled.code";
    
    // log
    EM_log( CK_LOG_FINE, "sporking %d %s...", count,
            count == 1 ? "instance" : "instances" );
    
    // spork it
    while( count-- )
    {
        // spork (for now, spork_immediate arg is always false)
        shred = m_carrier->vm->spork( vm_code, NULL, FALSE );
        // add args
        shred->args = args;
    }
    
    // pop indent
    EM_poplog();
    
    return true;
}




//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool ChucK::start()
{
    // sanity check
    if( m_carrier->vm == NULL )
    {
        CK_FPRINTF_STDERR( "[chuck]: cannot start, VM not initialized...\n" );
        return false;
    }
    
    // start the VM!
    if( !m_carrier->vm->running() ) m_carrier->vm->start();

    // return state
    return m_carrier->vm->running();
}




//-----------------------------------------------------------------------------
// run engine (call from callback)
//-----------------------------------------------------------------------------
void ChucK::run( SAMPLE * input, SAMPLE * output, int numFrames )
{
    // make sure we started...
    if( !m_carrier->vm->running() ) this->start();
}




//-----------------------------------------------------------------------------
// name: setExternalInt
// desc: send a message to set the value of an external int
//-----------------------------------------------------------------------------
t_CKBOOL ChucK::setExternalInt( const char * name, t_CKINT val )
{
    if( !m_carrier->vm->running() ) return FALSE;
    return !m_carrier->vm->set_external_int( std::string( name ), val );
}




//-----------------------------------------------------------------------------
// name: getExternalInt
// desc: send a message to get the value of an external int via callback
//-----------------------------------------------------------------------------
t_CKBOOL ChucK::getExternalInt( const char * name, void (* callback)(t_CKINT) )
{
    if( !m_carrier->vm->running() ) return FALSE;
    return !m_carrier->vm->get_external_int( std::string( name ), callback );
}




//-----------------------------------------------------------------------------
// name: setExternalFloat
// desc: send a message to set the value of an external float
//-----------------------------------------------------------------------------
t_CKBOOL ChucK::setExternalFloat( const char * name, t_CKFLOAT val )
{
    if( !m_carrier->vm->running() ) return FALSE;
    return !m_carrier->vm->set_external_float( std::string( name ), val );
}




//-----------------------------------------------------------------------------
// name: getExternalFloat
// desc: send a message to get the value of an external float via callback
//-----------------------------------------------------------------------------
t_CKBOOL ChucK::getExternalFloat( const char * name, void (* callback)(t_CKFLOAT) )
{
    if( !m_carrier->vm->running() ) return FALSE;
    return !m_carrier->vm->get_external_float( std::string( name ), callback );
}




//-----------------------------------------------------------------------------
// name: signalExternalEvent
// desc: send a message to signal an external event
//-----------------------------------------------------------------------------
t_CKBOOL ChucK::signalExternalEvent( const char * name )
{
    if( !m_carrier->vm->running() ) return FALSE;
    return !m_carrier->vm->signal_external_event( std::string( name ) );
}




//-----------------------------------------------------------------------------
// name: broadcastExternalEvent
// desc: send a message to broadcast an external event
//-----------------------------------------------------------------------------
t_CKBOOL ChucK::broadcastExternalEvent( const char * name )
{
    if( !m_carrier->vm->running() ) return FALSE;
    return !m_carrier->vm->broadcast_external_event( std::string( name ) );
}




//-----------------------------------------------------------------------------
// name: setChoutCallback
// desc: provide a callback where Chout print statements are routed
//-----------------------------------------------------------------------------
t_CKBOOL ChucK::setChoutCallback( void (* callback)(const char *) )
{
    if( !m_init || m_carrier->chout == NULL ) return FALSE;
    m_carrier->chout->set_output_callback( callback );
    return TRUE;
}




//-----------------------------------------------------------------------------
// name: setCherrCallback
// desc: provide a callback where Cherr print statements are routed
//-----------------------------------------------------------------------------
t_CKBOOL ChucK::setCherrCallback( void (* callback)(const char *) )
{
    if( !m_init || m_carrier->cherr == NULL ) return FALSE;
    m_carrier->cherr->set_output_callback( callback );
    return TRUE;
}




//-----------------------------------------------------------------------------
// name: setStdoutCallback
// desc: provide a callback where stdout print statements are routed
//-----------------------------------------------------------------------------
t_CKBOOL ChucK::setStdoutCallback( void (* callback)(const char *) )
{
    ck_set_stdout_callback( callback );
    return TRUE;
}




//-----------------------------------------------------------------------------
// name: setStderrCallback
// desc: provide a callback where <<< >>> and stdout print statements are routed
//-----------------------------------------------------------------------------
t_CKBOOL ChucK::setStderrCallback( void (* callback)(const char *) )
{
    ck_set_stderr_callback( callback );
    return TRUE;
}




//-----------------------------------------------------------------------------
// name: finalCleanup
// desc: provide a callback where <<< >>> and stdout print statements are routed
//-----------------------------------------------------------------------------
void ChucK::finalCleanup()
{
    // REFACTOR-2017 TODO Ge:
    // stop le_cb, ...?
}




//-----------------------------------------------------------------------------
// name: poop()
// desc: --poop
//-----------------------------------------------------------------------------
void ChucK::poop()
{
    ::uh();
}