#include "settings.h"

int readSettings(const char* configFilePath)
{    
    config_setting_t *setting;    

    config_init(&cfg);
  
    /* Read the file. If there is an error, report it and exit. */
    if(!config_read_file(&cfg, configFilePath))
    {
        fprintf(stderr, "%s:%d - %s\n", config_error_file(&cfg),
                config_error_line(&cfg), config_error_text(&cfg));
        config_destroy(&cfg);
        return(EXIT_FAILURE);
    }
    
    if(!config_lookup_int(&cfg, "CONNECTION_BACKLOG", &CONNECTION_BACKLOG))        
    {
        fprintf(stderr, "No 'CONNECTION_BACKLOG' setting in configuration file.\n");
        return(EXIT_FAILURE); 
    }

    if(!config_lookup_string(&cfg, "addr", &addr))        
    {
        fprintf(stderr, "No 'addr' setting in configuration file.\n");
        return(EXIT_FAILURE); 
    }    

    if(!config_lookup_int(&cfg, "port", &port))        
    {
        fprintf(stderr, "No 'port' setting in configuration file.\n");
        return(EXIT_FAILURE); 
    }
    
    if(!config_lookup_int(&cfg, "debug", &debug))        
    {
        fprintf(stderr, "No 'debug' setting in configuration file.\n");
        return(EXIT_FAILURE); 
    }
    
    if(!config_lookup_int(&cfg, "waitinactive", &waitinactive))        
    {
        fprintf(stderr, "No 'waitinactive' setting in configuration file.\n");
        return(EXIT_FAILURE); 
    }
    
    if(!config_lookup_int(&cfg, "oftentime", &oftentime))        
    {
        fprintf(stderr, "No 'oftentime' setting in configuration file.\n");
        return(EXIT_FAILURE); 
    }
    
    if(!config_lookup_int(&cfg, "betweentimeout", &betweentimeout))        
    {
        fprintf(stderr, "No 'betweentimeout' setting in configuration file.\n");
        return(EXIT_FAILURE); 
    }
    
    if(!config_lookup_string(&cfg, "primarypglogin", &primarypglogin))        
    {
        fprintf(stderr, "No 'primarypglogin' setting in configuration file.\n");
        return(EXIT_FAILURE); 
    }

    if(!config_lookup_string(&cfg, "primarypgpwd", &primarypgpwd))        
    {
        fprintf(stderr, "No 'primarypgpwd' setting in configuration file.\n");
        return(EXIT_FAILURE); 
    }
    
    if(!config_lookup_string(&cfg, "primarypghost", &primarypghost))        
    {
        fprintf(stderr, "No 'primarypghost' setting in configuration file.\n");
        return(EXIT_FAILURE); 
    }
    
    if(!config_lookup_string(&cfg, "primarypgport", &primarypgport))        
    {
        fprintf(stderr, "No 'primarypgport' setting in configuration file.\n");
        return(EXIT_FAILURE); 
    }
    
    if(!config_lookup_string(&cfg, "primarydbname", &primarydbname))        
    {
        fprintf(stderr, "No 'primarydbname' setting in configuration file.\n");
        return(EXIT_FAILURE); 
    }
    
    if(!config_lookup_int(&cfg, "primarytimeout", &primarytimeout))        
    {
        fprintf(stderr, "No 'primarytimeout' setting in configuration file.\n");
        return(EXIT_FAILURE); 
    }

    if(!config_lookup_string(&cfg, "primaryssl", &primaryssl))        
    {
        fprintf(stderr, "No 'primaryssl' setting in configuration file.\n");
        return(EXIT_FAILURE); 
    }
    
    if(!config_lookup_string(&cfg, "secondarypglogin", &secondarypglogin))        
    {
        fprintf(stderr, "No 'secondarypglogin' setting in configuration file.\n");
        return(EXIT_FAILURE); 
    }

    if(!config_lookup_string(&cfg, "secondarypgpwd", &secondarypgpwd))        
    {
        fprintf(stderr, "No 'secondarypgpwd' setting in configuration file.\n");
        return(EXIT_FAILURE); 
    }
    
    if(!config_lookup_string(&cfg, "secondarypghost", &secondarypghost))        
    {
        fprintf(stderr, "No 'secondarypghost' setting in configuration file.\n");
        return(EXIT_FAILURE); 
    }

    if(!config_lookup_string(&cfg, "secondarypgport", &secondarypgport))        
    {
        fprintf(stderr, "No 'secondarypgport' setting in configuration file.\n");
        return(EXIT_FAILURE); 
    }

    if(!config_lookup_string(&cfg, "primarypgport", &primarypgport))        
    {
        fprintf(stderr, "No 'primarypgport' setting in configuration file.\n");
        return(EXIT_FAILURE); 
    }
    
    if(!config_lookup_int(&cfg, "secondarytimeout", &secondarytimeout))        
    {
        fprintf(stderr, "No 'secondarytimeout' setting in configuration file.\n");
        return(EXIT_FAILURE); 
    }

    if(!config_lookup_string(&cfg, "secondaryssl", &secondaryssl))        
    {
        fprintf(stderr, "No 'secondaryssl' setting in configuration file.\n");
        return(EXIT_FAILURE); 
    }
        
    if(!config_lookup_string(&cfg, "transferaddr", &transferaddr))        
    {
        fprintf(stderr, "No 'transferaddr' setting in configuration file.\n");
        return(EXIT_FAILURE); 
    }

    if(!config_lookup_int(&cfg, "transferport", &transferport))        
    {
        fprintf(stderr, "No 'transferport' setting in configuration file.\n");
        return(EXIT_FAILURE); 
    }

    if(!config_lookup_int(&cfg, "isdaemon", &isdaemon))        
    {
        fprintf(stderr, "No 'isdaemon' setting in configuration file.\n");
        return(EXIT_FAILURE); 
    }
    
    if(!config_lookup_string(&cfg, "pidfile", &pidfile))        
    {
        fprintf(stderr, "No 'pidfile' setting in configuration file.\n");
        return(EXIT_FAILURE); 
    }
    
    if(!config_lookup_int(&cfg, "usememcache", &usememcache))        
    {
        fprintf(stderr, "No 'usememcache' setting in configuration file.\n");
        return(EXIT_FAILURE); 
    }
    
    if(CONNECTION_BACKLOG <= 0)
    {
      CONNECTION_BACKLOG = 1;
    }
    if(CONNECTION_BACKLOG > 100)
    {
      CONNECTION_BACKLOG = 100;
    }
            
    return EXIT_SUCCESS;
}
