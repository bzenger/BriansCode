function ProcessingScript(varargin)

% FUNCTION ProcessingScript
%
% DESCRIPTION
% This is the main entry point into the GUI processing software.
% This script will lead you through a couple of steps to do the
% basic signal processing, starting from ACQ files and ending up
% with tsdf files (timesseries, integration maps and, activation/recovery maps).
%
% INPUT
% none (everything is gui based)
%
% OUTPUT
% lots of files hopefully
%
% DATA FILES:
% processingscript.mat  - Last settings used to process some data.
% processingdata.mat    - A database with all the settings and selections the
%						user made, so reprocessing a file becomes easy!
%

    if nargin > 1,
        feval(varargin{1},varargin{2:end});
    else
        if nargin == 1,
            Init(varargin{1});
        else
            Init;
        end
    end
return

function Init(filename)

    global SCRIPT SCRIPTDATA;
    InitScript;
    InitScriptData;
    
    if nargin == 0,
        filename = SCRIPT.SCRIPTFILE;
    end
    
    script = [];
    filename = fullfile(pwd,filename);
    if exist(filename,'file'),
        load(filename,'-mat');
        script = rmfield(script,'TYPE');
        script = rmfield(script,'DEFAULT');
    
        if exist('script','var'),
            fn = fieldnames(script);
            for p = 1:length(fn),
                SCRIPT = setfield(SCRIPT,fn{p},getfield(script,fn{p}));
            end
        end
      
        
        fprintf(1,'Looking for ACQ or AC2 Files in this directory\n');

        GetACQLabel;
        LoadScriptData;
        
        handle = winProcessingScriptMenu2;
        UpdateACQFiles(handle);        
        
    else
        
        GetACQLabel;
        LoadScriptData;
        
        fprintf(1,'Looking for ACQ or AC2 Files in this directory\n');
        handle = winProcessingScriptMenu2;
        UpdateACQFiles(handle);        
     end
    
    UpdateGroup;
    handle = winProcessingScriptSettings2;
    UpdateFigure(handle);
    handle = winProcessingScriptMenu2;
    UpdateFigure(handle);
    
    SCRIPT.PWD = pwd;
    
    return
    
    
function InitScript

%%% DEFAULT SETUP %%%%

    defaultsettings = { 'PWD','','file',...
                    'MAPPINGFILE','','file', ...
                    'PACINGLEAD',[],'double', ...
                    'CALIBRATIONFILE','','file', ...
                    'CALIBRATIONACQ','','vector', ...
                    'CALIBRATIONACQUSED','','vector',...
                    'CALIBRATIONMAPPINGUSED','','file',...
                    'SCRIPTFILE','processingscript.mat','file',...
                    'ACQLABEL','','string',...
                    'ACQLISTBOX','','listbox',...
                    'ACQFILES',[],'listboxedit',...
                    'ACQPATTERN','','string',...
                    'ACQFILENUMBER',[],'vector',...
                    'ACQINFO',{},'string',...
                    'ACQFILENAME',{},'string',...
                    'ACQNUM',0,'integer',...
                    'DATAFILE','processingdata.mat','file',...
                    'TSDFDIR','autoprocessing','file',...
                    'ACQDIR','','file',...
                    'ACQCONTAIN','','string',...
                    'ACQCONTAINNOT','','string',...
                    'ACQEXT','.acq,.ac2','string',...
                    'BASELINEWIDTH',5,'integer',...
                    'GROUPNAME','GROUP','groupstring',...
                    'GROUPLEADS',[],'groupvector',...
                    'GROUPEXTENSION','-ext','groupstring',...
                    'GROUPTSDFC','group.tsdfc','groupfile',...
                    'GROUPGEOM','','groupfile',...
                    'GROUPCHANNELS','','groupfile',...
                    'GROUPBADLEADSFILE','','groupfile',...
                    'GROUPBADLEADS',[],'groupvector',...
                    'GROUPDONOTPROCESS',0,'groupbool',...
                    'GROUPDONOTDISPLAY',0,'groupbool',...
                    'GROUPSELECT',0,'select',...
                    'DO_CALIBRATE',1,'bool',...
                    'DO_BLANKBADLEADS',1,'bool',...
                    'DO_SLICE',1,'bool',...
                    'DO_SLICE_USER',1,'bool',...
                    'DO_ADDBADLEADS',0,'bool',...
                    'DO_SPLIT',1,'bool',...
                    'DO_BASELINE',1,'bool',...
                    'DO_BASELINE_RMS',0,'bool',...
                    'DO_BASELINE_USER',1,'bool',...
                    'DO_DELTAFOVERF',0,'bool',...
                    'DO_DETECT',1,'bool',...
                    'DO_DETECT_USER',1,'bool',...
                    'DO_DETECT_LOADTSDFC',1,'bool',...
                    'DO_DETECT_AUTO',1,'bool',...
                    'DO_DETECT_PACING',1,'bool',...
                    'DO_LAPLACIAN_INTERPOLATE',1,'bool',...
                    'DO_INTERPOLATE',0,'bool',...
                    'DO_INTEGRALMAPS',1,'bool',...
                    'DO_ACTIVATION',0,'bool',...
                    'DO_ACTIVATIONMAPS',1,'bool',...
                    'DO_FILTER',0,'bool',...
                    'NAVIGATION','apply','string',...
                    'DISPLAYTYPE',1,'integer',...
                    'DISPLAYTYPEF',1,'integer',...
                    'DISPLAYSCALING',1,'integer',...
                    'DISPLAYSCALINGF',1,'integer',...
                    'DISPLAYOFFSET',1,'integer',...
                    'DISPLAYGRID',1,'integer',...
                    'DISPLAYGRIDF',1,'integer',...
                    'DISPLAYLABEL',1,'integer',...
                    'DISPLAYLABELF',1,'integer',...
                    'DISPLAYTYPEF1',1,'integer',...
                    'DISPLAYTYPEF2',1,'integer',...
                    'DISPLAYPACING',1,'integer',...
                    'DISPLAYPACINGF',1,'integer',...
                    'DISPLAYGROUP',1,'vector',...
                    'DISPLAYGROUPF',1,'vector',...
                    'DISPLAYSCALE',1,'integer',...
                    'CURRENTTS',1,'integer',...
                    'ALIGNSTART','detect','integer',...
                    'ALIGNSIZE','detect','integer',...
                    'ALIGNMETHOD',1,'integer',...
                    'ALIGNSTARTENABLE',1,'integer',...
                    'ALIGNSIZEENABLE',1,'integer',...
                    'ALIGNRMSTYPE',1,'integer',...
                    'ALIGNTHRESHOLD',0.9,'double',...
                    'AVERAGEMETHOD',1,'integer',...
                    'AVERAGERMSTYPE',1,'integer',...
                    'AVERAGECHANNEL',1,'integer',...
                    'AVERAGEMAXN',5,'integer',...
                    'AVERAGEMAXRE',0.1,'double',...
                    'KEEPBADLEADS',1,'integer',...
                    'FIDSLOOPFIDS',1,'integer',...
                    'FIDSAUTOACT',1,'integer',...
                    'FIDSAUTOREC',1,'integer',...
                    'FIDSAUTOPEAK',1,'integer',...
                    'FIDSACTREV',0,'integer',...
                    'FIDSRECREV',0,'integer',...
                    'TSDFODIR','tsdf','string',...
                    'TSDFODIRON',1,'bool',...
                    'MATODIR','mat','string',...
                    'MATODIRON',0,'bool',...
                    'SCIMATODIR','scimat','string',...
                    'SCIMATODIRON',0,'bool',...
                    'ACTWIN',7,'integer',...
                    'ACTDEG',3,'integer',...
                    'ACTNEG',1,'integer',...
                    'RECWIN',7,'integer',...
                    'RECDEG',3,'integer',...
                    'RECNEG',0,'integer',...
                    'ALEADNUM',1,'integer',...
                    'ADOFFSET',0.2,'double',...
                    'ADISPLAYTYPE',1,'integer',...
                    'ADISPLAYOFFSET',1,'integer',...
                    'ADISPLAYGRID',1,'integer',...
                    'ADISPLAYGROUP',1,'vector',...
                    'OPTICALLABEL','','string',...
                    'FILTERFILE','','string',...
                    'FILTERNAME','NONE','string',...
                    'FILTERNAMES',{'NONE'},'string',...
                    'FILTER',[],'string',...
                    'INPUTTSDFC','','string'
            };
    global SCRIPT;

    SCRIPT = [];
    SCRIPT.TYPE = [];
    SCRIPT.DEFAULT = [];

    for p=1:3:length(defaultsettings),
        if strncmp(defaultsettings{p+2},'group',5) == 1,
            SCRIPT = setfield(SCRIPT,defaultsettings{p},{});
        else
            SCRIPT = setfield(SCRIPT,defaultsettings{p},defaultsettings{p+1});
        end
        SCRIPT.TYPE = setfield(SCRIPT.TYPE,defaultsettings{p},defaultsettings{p+2});
        SCRIPT.DEFAULT = setfield(SCRIPT.DEFAULT,defaultsettings{p},defaultsettings{p+1});
    end

    return
    
    
function InitScriptData
    
    % THIS FUNCTION INITIALISES THE SCRIPTDATA STRUCTURE
    % SCRIPTDATA CONTAINS THE DATA THE AUTOPROCESSING PROCEDURE
    % CANNOT STORE IN THE NORMAL FILES. CURRENTLY TWO SETS OF DATA
    % ARE STORED: SELFRAMES -> THE WINDOW OF THE SELECTED BEAT
    %             REFFRAMES -> THE CURRENT REFERENCE FRAME FOR THE
    %                          FIDUCIALS
    % THE ACQ LABEL IS STORED AS WELL. THE LATTER IS TO INSURE THAT
    % THE SELFRAMES AND REFFRAMES BELONG TO THIS DATASET

    global SCRIPTDATA;
    
    SCRIPTDATA = [];
    SCRIPTDATA.ACQLABEL = '';
    SCRIPTDATA.SELFRAMES = {};
    SCRIPTDATA.REFFRAMES = {};
    SCRIPTDATA.AVERAGESTART = {};
    SCRIPTDATA.AVERAGEEND = {};
    
    return
    
function SaveScriptData

    % THIS FUNCTION SAVES THE SCRIPTDATA TO DISK

    global SCRIPT SCRIPTDATA;
    
    scriptdata = SCRIPTDATA;
    if (sscanf(version,'%d') < 7)
       save(SCRIPT.DATAFILE,'scriptdata','-mat');
    else
       save(SCRIPT.DATAFILE,'scriptdata','-mat','-v6');
    end
    return
    
function LoadScriptData

    % LOAD THE SCRIPTDATA FROM FILE

    global SCRIPT SCRIPTDATA;
    
    if exist(SCRIPT.DATAFILE,'file'),
        load(SCRIPT.DATAFILE,'-mat');
    end
    
    % THE OLD FORMAT HAS A FIELD ACQLABEL TO CHECK WHETHER THE DATA IS ACCURATE
    % THE NEW FORMAT HAS THE COMPLETE FILENAME (BASED ON THE OLD LABEL THE FILENAMES
    % ARE RECONSTRUCTED HERE). IN THE NEW FILES ACQLABEL WILL NO LONGER
    % EXIST!!!
    
    
    if exist('scriptdata','var'),
        if (~isfield(scriptdata,'ACQLABEL')|(isempty(scriptdata.ACQLABEL))),
            fields = fieldnames(scriptdata);
            for p=1:length(fields),
                SCRIPTDATA = setfield(SCRIPTDATA,fields{p},getfield(scriptdata,fields{p}));
            end
        elseif (strcmp(SCRIPT.ACQLABEL,scriptdata.ACQLABEL) == 1),
            fields = fieldnames(scriptdata);
            fllength = 1;
            for p=1:length(fields),
                SCRIPTDATA = setfield(SCRIPTDATA,fields{p},getfield(scriptdata,fields{p}));
                if length(getfield(scriptdata,fields{p})) > fllength, fllength = length(getfield(scriptdata,fields{p})); end   
            end
            
            % CONVERSION TO NEWER FORMAT
            
            acqlabel = scriptdata.ACQLABEL;
            filename = {};
            for p=1:fllength,
                filename{p} = sprintf('%s-%04d.acq',acqlabel,p);
            end
            SCRIPTDATA.FILENAME = filename;
            SCRIPTDATA = rmfield(SCRIPTDATA,'ACQLABEL');
        end
    end
    
    if ~isfield(SCRIPTDATA,'FILENAME'),   
        SCRIPTDATA.FILENAME = {};
    end
    return
    
function vec = mystr2num(str)
    
    vec = eval(['[' str ']']);
    return

function str = mynum2str(vec)

    if length(vec) == 1,
        str = num2str(vec);
    else
        if nnz(vec-round(vec)) > 0,
            str = num2str(vec);
        else
            vec = sort(vec);
            str = '';
            ind = 1;
            len = length(vec);
            while (ind <= len),
                if (len-ind) > 0,
                     step = vec(ind+1)-vec(ind);
                     k = 1;
                     while (k+ind+1 <= len)
                         if vec(ind+k+1)-vec(ind+k) == step, 
                             k = k + 1;
                         else
                             break;
                         end
                     end
                     if k > 1,
                         if step == 1,
                            str = [str sprintf('%d:%d ',vec(ind),vec(ind+k))]; ind = ind + k+1;
                        else
                            str = [str sprintf('%d:%d:%d ',vec(ind),step,vec(ind+k))]; ind = ind + k+1;
                        end
                     else
                         str = [str sprintf('%d ',vec(ind))]; ind = ind + 1;
                     end
                 else
                     for p = ind:len,
                         str = [str sprintf('%d ',vec(p))]; ind = len + 1;
                     end
                 end
             end
         end
     end
     return
    
function strs = commalist(str)

    str = str(find(isspace(str)==0));
    ind = [0 sort([findstr(',',str) findstr(';',str)]) length(str)+1];
    for p=1:(length(ind)-1),
        strs{p} = str((ind(p)+1):(ind(p+1)-1));
    end
    
    return
    

function UpdateFigure(handle)

    global SCRIPT;
    
    if isempty(SCRIPT),
        InitScript;    
    end
    
    fn = fieldnames(SCRIPT);
    for p=1:length(fn),
        obj = findobj(allchild(handle),'tag',fn{p});
        if ~isempty(obj),
            objtype = getfield(SCRIPT.TYPE,fn{p});
            switch objtype,
                case {'file','string'},
                    set(obj,'string',getfield(SCRIPT,fn{p}));
                case {'listbox'},
                    cellarray = getfield(SCRIPT,fn{p});
                    if ~isempty(cellarray), 
                        values = intersect(SCRIPT.ACQFILENUMBER,SCRIPT.ACQFILES);
                        set(obj,'string',cellarray,'max',length(cellarray),'value',values,'enable','on');
                    else
                        set(obj,'string',{'NO ACQ or AC2 FILES FOUND','',''},'max',3,'value',[],'enable','off');
                    end
                case {'double','vector','listboxedit','integer'},
                    set(obj,'string',mynum2str(getfield(SCRIPT,fn{p})));
                case {'bool'},
                    set(obj,'value',getfield(SCRIPT,fn{p}));
                case {'select'},
                    value = getfield(SCRIPT,fn{p});
                    if value == 0, value = 1; end
                    set(obj,'value',value);
                    selectnames = SCRIPT.GROUPNAME;
                    selectnames{end+1} = 'NEW GROUP';
                    set(obj,'string',selectnames);
                case {'groupfile','groupstring','groupdouble','groupvector','groupbool'},
                    group = SCRIPT.GROUPSELECT;
                    if (group > 0),
                        set(obj,'enable','on','visible','on');
                        cellarray = getfield(SCRIPT,fn{p});
                        if length(cellarray) < group,
                            cellarray{group} = getfield(SCRIPT.DEFAULT,fn{p});
                        end
                        switch objtype(6:end),
                            case {'file','string'},
                                set(obj,'string',cellarray{group});
                            case {'double','vector','integer'},
                                set(obj,'string',mynum2str(cellarray{group}));
                            case {'bool'},
                                set(obj,'value',cellarray{group});
                        end
                        SCRIPT = setfield(SCRIPT,fn{p},cellarray);
                    else
                        set(obj,'enable','inactive','visible','off');
                    end
            end
        end
    end
    return
    
function LoadSettings(handle)
    
    global SCRIPT;
    
    [filename,pathname] = uigetfile('*.mat','Choose processing script file');
    SCRIPT.SCRIPTFILE = fullfile(pathname,filename);
    filename = SCRIPT.SCRIPTFILE;
    
    script = [];
    if exist(filename,'file'),
        load(filename,'-mat');
        script = rmfield(script,'TYPE');
        script = rmfield(script,'DEFAULT');
    
        
        if exist('script','var'),
    
            % COMPATIBILITY CONVERSIONS
            
            % RENAMED THIS ONE TO BE MORE CONSISTENT
            if ~isfield(script,'TSDFODIR'),
                script.TSDFODIR = script.TSDFDIR;
            end
            
            fn = fieldnames(script);
            for p = 1:length(fn),
                SCRIPT = setfield(SCRIPT,fn{p},getfield(script,fn{p}));
            end
        end
 
        UpdateACQFiles(handle);
        
        %d = dir(sprintf('%s-*.acq',SCRIPT.ACQLABEL));
        %if length(d) == 0, 
        %    GetACQLabel;
        %    fprintf(1,'Looking for ACQ Files in this directory\n');
        %    GetACQFiles;        
        %end
        
    end        

    UpdateGroup;
    UpdateFigure(findobj(allchild(0),'tag','PROCESSINGSCRIPTSETTINGS'));
    UpdateFigure(findobj(allchild(0),'tag','PROCESSINGSCRIPTMENU'));

    return
    
function SaveSettings(handle)

    global SCRIPT;
    filename = SCRIPT.SCRIPTFILE;
    
    script = SCRIPT;
    if (sscanf(version,'%d')<7)
        save(filename,'script','-mat');
    else
        save(filename,'script','-mat','-v6');
    end
    return;
    
function RemoveGroup(handle)

    global SCRIPT;
    group = SCRIPT.GROUPSELECT;
    if group > 0,
       fn = fieldnames(SCRIPT.TYPE);
       for p=1:length(fn),
           if strncmp(fn{p},'GROUP',5) == 1,
               cellarray = getfield(SCRIPT,fn{p});
               ind = 1:length(cellarray);
               ind = ind(find(ind ~= group));
               cellarray = cellarray(ind);
               SCRIPT = setfield(SCRIPT,fn{p},cellarray);
           end
       end
       SCRIPT.GROUPSELECT = 0;
   end
   
   UpdateFigure(handle);
   return
    
function SetScript(handle)

    global SCRIPT;
   
    tag = get(handle,'tag');
    if isfield(SCRIPT.TYPE,tag),
        objtype = getfield(SCRIPT.TYPE,tag);
    else
        objtype = 'string';
    end
    switch objtype
        case {'file','string'},
            SCRIPT = setfield(SCRIPT,tag,get(handle,'string'));
            if strcmp(tag,'GROUPNAME') == 1,
            end
        case {'double','vector','integer'},
            SCRIPT = setfield(SCRIPT,tag,mystr2num(get(handle,'string')));
        case 'bool'
            SCRIPT = setfield(SCRIPT,tag,get(handle,'value'));
        case 'select'
            SCRIPT = setfield(SCRIPT,tag,get(handle,'value'));
        case 'listbox'
            SCRIPT.ACQFILES = SCRIPT.ACQFILENUMBER(get(handle,'value'));
        case {'listboxedit'}
            SCRIPT = setfield(SCRIPT,tag,mystr2num(get(handle,'string')));
        case {'groupfile','groupstring','groupdouble','groupvector','groupbool'},
            group = SCRIPT.GROUPSELECT;
            if (group > 0),
                if isfield(SCRIPT,tag),
                    cellarray = getfield(SCRIPT,tag);
                else
                    cellarray = {};
                end
                switch objtype(6:end)
                    case {'file','string'}
                        cellarray{group} = get(handle,'string');
                    case {'double','vector'}
                        cellarray{group} = mystr2num(get(handle,'string'));
                    case {'bool'}
                        cellarray{group} = get(handle,'value');
                end
                SCRIPT = setfield(SCRIPT,tag,cellarray);
            end
    end
    
    UpdateFigure(findobj(allchild(0),'tag','PROCESSINGSCRIPTSETTINGS'));
    UpdateFigure(findobj(allchild(0),'tag','PROCESSINGSCRIPTMENU'));    

    return

function SelectFilter(handle)
    
    global SCRIPT;
    
    tag = get(handle,'tag');
    
    switch tag,
        case 'FILTERNAMES',
            val = get(handle,'value');
            if val <= length(SCRIPT.FILTERNAMES)
                SCRIPT.FILTERNAME = SCRIPT.FILTERNAMES{val};
            else
                SCRIPT.FILTERNAME = '';
            end
        case 'FILTERFILE',
            fn = get(handle,'string');
            if exist(fn,'file'),
                try
                    load(fn,'-mat');
                    SCRIPT.FILTER = session.Filters;
                    SCRIPT.FILTERNAMES = {};
                    for p=1:length(session.Filters),
                        SCRIPT.FILTERNAMES{p} = session.Filters(p).label;
                    end
                    if length(session.Filters) > 0,
                        SCRIPT.FILTERNAME = SCRIPT.FILTERNAMES{1};
                        SCRIPT.FILTER = session.Filters;
                    else
                        SCRIPT.FILTER = [];
                        SCRIPT.FILTERNAMES = {'NONE'};
                        SCRIPT.FILTERNAME = 'NONE';                      
                    end
                catch
                    SCRIPT.FILTER = [];
                    SCRIPT.FILTERNAMES = {'NONE'};
                    SCRIPT.FILTERNAME = 'NONE';
                end
                win = findobj(allchild(0),'tag','PROCESSINGSCRIPTSETTINGS');
                shandle = findobj(allchild(win),'tag','FILTERNAMES');
                set(shandle,'string',SCRIPT.FILTERNAMES,'value',1);
            end
    end
            
    return
    
function Browse(handle,ext,mode)

    global SCRIPT;
    if nargin == 1,
        ext = 'mat';
        mode = 'file';
    end
    
    if nargin == 2,
        mode = 'file';
    end
    
    tag = get(handle,'tag');
    tag = tag(8:end);
    
    filename = getfield(SCRIPT,tag);
    
    switch mode,
        case 'file',
            [fn,pn] = uigetfile(['*.' ext],'SELECT FILE',filename);
            if (fn == 0), return; end
            SCRIPT = setfield(SCRIPT,tag,fullfile(pn,fn));
        case 'dir',
            pn  = uigetdir(pwd,'SELECT DIRECTORY');
            if (pn == 0), return; end
            SCRIPT = setfield(SCRIPT,tag,pn);         
    end
    
    parent = get(handle,'parent');
    UpdateFigure(parent);
    
    if strcmp(ext,'spt')
        handle = findobj(allchild(parent),'tag','FILTERFILE');
        fn = get(handle,'string');
            if exist(fn,'file'),
                try
                    load(fn,'-mat');
                    SCRIPT.FILTER = session.Filters;
                    SCRIPT.FILTERNAMES = {};
                    for p=1:length(session.Filters)
                        SCRIPT.FILTERNAMES{p} = session.Filters(p).label;
                    end
                    if length(session.Filters) > 0,
                        SCRIPT.FILTERNAME = SCRIPT.FILTERNAMES{1};
                        SCRIPT.FILTER = session.Filters;
                    else
                        SCRIPT.FILTER = [];
                        SCRIPT.FILTERNAMES = {'NONE'};
                        SCRIPT.FILTERNAME = 'NONE';                      
                    end
                catch
                    SCRIPT.FILTER = [];
                    SCRIPT.FILTERNAMES = {'NONE'};
                    SCRIPT.FILTERNAME = 'NONE';
                end
                win = findobj(allchild(0),'tag','PROCESSINGSCRIPTSETTINGS');
                shandle = findobj(allchild(win),'tag','FILTERNAMES');
                set(shandle,'string',SCRIPT.FILTERNAMES,'value',1);
            end    
    end
    
    return
    
function UpdateACQFiles(handle)

    GetACQLabel;
    GetACQFiles;
    UpdateFigure(findobj(allchild(0),'tag','PROCESSINGSCRIPTMENU'));
    return
    
function GetACQLabel

    global SCRIPT;
    
    olddir = pwd;
    if ~isempty(SCRIPT.ACQDIR),
        if exist(SCRIPT.ACQDIR,'dir');
            cd(SCRIPT.ACQDIR);
        end
    end
    
    d = dir('*.acq');
    if length(d) == 0
        d = dir('*.ac2');
        if length(d) == 0,
            cd(olddir); 
            return; 
        end
    end
    
    for p=1:length(d), t{p} = d(p).name(1:(findstr(d(p).name,'-')-1)); end
    [B,I,J] = unique(t);
    [dummy,I] = max(hist(J,1:length(B)));
    
    label = t{I};
    SCRIPT.ACQLABEL = label;
    
    cd(olddir);
    
    return

function GetACQFiles

    global SCRIPT;
   
    oldfilenames = {};
    if ~isempty(SCRIPT.ACQFILES),
        for p=1:length(SCRIPT.ACQFILES),
            if SCRIPT.ACQFILES(p) <= length(SCRIPT.ACQFILENAME),
                oldfilenames{end+1} = SCRIPT.ACQFILENAME{SCRIPT.ACQFILES(p)};
            end
        end
    end
    
    exts = commalist(SCRIPT.ACQEXT);
    contain = commalist(SCRIPT.ACQCONTAIN);
    containnot = commalist(SCRIPT.ACQCONTAINNOT);
    
    olddir = pwd;
    if ~isempty(SCRIPT.ACQDIR),
        if exist(SCRIPT.ACQDIR,'dir');
            if ~isempty(dir(SCRIPT.ACQDIR)),
                cd(SCRIPT.ACQDIR);
            end
        end
    end
    
    filenames = {};
    for p=1:length(exts),
        d = dir(sprintf('*%s',exts{p}));
        for q= 1:length(d),
            filenames{end+1} = d(q).name;
        end
    end
    
    filterfilenames = {};
    
    if (length(contain)==1), if isempty(contain{1}), contain = {}; end, end
    if (length(containnot)==1), if isempty(containnot{1}), containnot = {}; end, end
    
    
    if ~isempty(contain),
        for p=1:length(contain),
            if isempty(contain{p}), continue; end
            indices = zeros(length(filenames));
            for q = 1:length(filenames),
                indices(q) = ~isempty(strfind(filenames{q},contain{p}));
            end
            indices = find(indices);
            filterfilenames((end+1):(end+length(indices))) = filenames(indices);
        end
    else
        filterfilenames = filenames;
    end
    
    if ~isempty(containnot),
        for p=1:length(containnot),
            if isempty(containnot{p}), continue, end
            indices = zeros(length(filterfilenames));
            for q = 1:length(filenames),
                indices(q) = ~isempty(strfind(filterfilenames{q},contain{p}));
            end
            indices = find(indices);
            filterfilenames(indices) = [];
        end
    end
    
    filenames = sort(unique(filterfilenames));
    
    options.scantsdffile = 1;
   
    SCRIPT.ACQFILENUMBER = [];
    SCRIPT.ACQLISTBOX= {};
    SCRIPT.ACQFILENAME = {};
    SCRIPT.ACQINFO = {};
    SCRIPT.ACQFILES = [];
    
    if isempty(filenames),
        cd(olddir)
        return
    end
    
    h = waitbar(0,'INDEXING AND READING FILES'); drawnow;
    T = ioReadTSdata(filenames,options);
    waitbar(0.8,h);
    
    for p = 1:length(T),
        if ~isfield(T{p},'time'), T{p}.time = 'none'; end
        if ~isfield(T{p},'label'), T{p}.label = 'no label'; end
        SCRIPT.ACQFILENUMBER(p) = p;
        SCRIPT.ACQLISTBOX{p} = sprintf('%04d % 35s   %12s  %40s',SCRIPT.ACQFILENUMBER(p),T{p}.filename,T{p}.time,T{p}.label);
        SCRIPT.ACQFILENAME{p} = T{p}.filename;
        SCRIPT.ACQINFO{p} = T{p}.label;
    end
    
    [dummy1,dummy2,SCRIPT.ACQFILES] = intersect(oldfilenames,SCRIPT.ACQFILENAME);
    SCRIPT.ACQFILES = sort(SCRIPT.ACQFILES);
    
    waitbar(1,h); drawnow;
    close(h);
    
    cd(olddir);
    return
    
function SelectNoneACQ(handle)

    global SCRIPT;
    SCRIPT.ACQFILES = [];
    UpdateFigure(findobj(allchild(0),'tag','PROCESSINGSCRIPTMENU'));
    
function SelectAllACQ(handle)

    global SCRIPT;
    SCRIPT.ACQFILES = SCRIPT.ACQFILENUMBER;
    UpdateFigure(findobj(allchild(0),'tag','PROCESSINGSCRIPTMENU'));
    
function ACQSelectLabel(handle)
    
    global SCRIPT;
    pat = SCRIPT.ACQPATTERN;
    sel = [];
    for p=1:length(SCRIPT.ACQINFO),
        if ~isempty(strfind(SCRIPT.ACQINFO{p},pat)), sel = [sel SCRIPT.ACQFILENUMBER(p)]; end
    end
    SCRIPT.ACQFILES = sel;
    UpdateFigure(findobj(allchild(0),'tag','PROCESSINGSCRIPTMENU'));
    
function LoadOldParameters(handle)

    % THIS FUNCTION SOLE PURPOSE IS TO GET THE OLD PARAMETERS FILES INTO
    % THE NEW SYSTEM
    
    
    global SCRIPT;
    
    if exist('parameters.m','file'),
        parameters;
        
        
        

        SCRIPT.ACQFILES = files;
        SCRIPT.MAPPINGFILE = mappingfile;
        SCRIPT.CALIBRATIONFILE = calfile;
        SCRIPT.CALIBRATIONACQ = [];
        if exist('acqcal','var'), SCRIPT.CALIBRATIONACQ = [SCRIPT.CALIBRATIONACQ acqcal]; end
        if exist('acq1mV','var'), SCRIPT.CALIBRATIONACQ = [SCRIPT.CALIBRATIONACQ acq1mV]; end
        if exist('acq10mV','var'), SCRIPT.CALIBRATIONACQ = [SCRIPT.CALIBRATIONACQ acq10mV]; end
        if exist('pacingchannel','var') SCRIPT.PACINGLEAD = pacingchannel; end
        
        for p=1:length(splitchannels), SCRIPT.GROUPNAME{p} = sprintf('GROUP %d',p); end
        SCRIPT.GROUPLEADS = splitchannels;
        SCRIPT.GROUPEXTENSION = splitext;
        SCRIPT.GROUPTSDFC = splittsdfcfiles;
        SCRIPT.GROUPBADLEADS = splitbadleads;
        
        if exist('lisurface','var'),
            for p=1:length(lisurface),
                surf = lisurface{p};
                for q=1:length(surf),
                    [pn,fn,ext] = fileparts(surf{q});
                    switch ext
                    case {'.geom','.fac'}
                        SCRIPT.GROUPGEOM{p} = [fn ext];
                    case '.channels'
                        SCRIPT.GROUPCHANNELS{p} = [fn ext];
                    end
                end
            end
        end
        
        UpdateGroup;
        %GetACQLabel;
        GetACQFiles;
        UpdateFigure(findobj(allchild(0),'tag','PROCESSINGSCRIPTSETTINGS'));
        UpdateFigure(findobj(allchild(0),'tag','PROCESSINGSCRIPTMENU'));
    end
    return

function UpdateGroup(handle)
    
    global SCRIPT;
    len = length(SCRIPT.GROUPNAME);
    fn = fieldnames(SCRIPT.TYPE);
    for p=1:length(fn),
        if strncmp(getfield(SCRIPT.TYPE,fn{p}),'group',5) == 1,
            cellarray = getfield(SCRIPT,fn{p});
            default = getfield(SCRIPT.DEFAULT,fn{p});
            if length(cellarray) < len, cellarray{len} = default; end
            for q=1:len,
                if isempty(cellarray{q}), cellarray{q} = default; end
            end
        end
    end
    
    return
                    
        
function RunScript(handle)

    % HANDLE IS THE CURRENT CALLBACK FIGURE
    % THIS VARIABLE IS NOT USED HERE IN ANYWAY

    global SCRIPT;    
    % IN THE GLOBAL ALL THE SCRIPT SETTINGS ARE KEPT
    
    SaveSettings(handle);
    % BEFORE RUNNING THE SCRIPT EVERYTHING IS SAVED, IN CASE THE PROGRAM
    % CRASHES WE CAN RESTART IT EASILY
    
    LoadScriptData;
    % THE DIFFERENCE BETWEEN SETTINGS AND DATA IS THE FOLLOWING SETTINGS
    % RECORD WHICH STEPS NEED TO BE PERFORMED IN THE SCRIPT, DATA RECORDS
    % WHAT THE USER SELECTED LAST TIME. DATA IS ON A FILE BY FILE BASIS,
    % WHEREAS SETTINGS ARE FOR THE WHOLE SCRIPT
    
    h = [];
    olddir =pwd;
    cd(SCRIPT.PWD);
    
    % TRY SHOULD BE INCLUDED IN FUTURE VERSIONS. THE PROBLEM IS GETTING
    % CLEAR ERROR MESSAGES, HENCE I DISABLED IT FOR THE MOMENT
    
   %try
        %% PRE LOOP BUSINESS %%
        % INCLUDES SETTING UP INTERPOLATION MATRICES, CALIBRATION FILES
        % ETC.
        PreLoopScript;
        SaveSettings(handle);
        
        if length(SCRIPT.GROUPNAME) == 0,
            errordlg('you need to define groups for processing the data');
            return;
        end
        
        %% MAIN LOOP %%%
    
        acqfiles = unique(SCRIPT.ACQFILES);
        h  = waitbar(0,'SCRIPT PROGRESS'); drawnow;
   
        p = 1;
        while (p <= length(acqfiles)),
            
            SCRIPT.ACQNUM = acqfiles(p);            
            ProcessACQFile(SCRIPT.ACQFILENAME{acqfiles(p)},SCRIPT.ACQDIR);
            
            % FIRST SIMPLE NAVIGATION TOOLS
            % THESE ARE NOT SOPHISTICATED BUT ALLOW
            % FOR A SIMPLE REDO AND GO BACK OPTION
            switch SCRIPT.NAVIGATION,
                case 'prev',
                    p = p-1; 
                    if p == 0, p = 1; end
                    continue;
                case {'redo','back'}
                    continue;
                case 'stop'
                    break;
            end
            waitbar(p/length(acqfiles),h);
            p = p+1;
        end
        
        close(h);
        
        %catch
        %cd(olddir);
        %errordlg(lasterr,'PROCESSING SCRIPT ERROR'); 
        %if ishandle(h), close(h); end
        %end
    cd(olddir);
    
    UpdateGroup;
    UpdateFigure(findobj(allchild(0),'tag','PROCESSINGSCRIPTSETTINGS'));
    UpdateFigure(findobj(allchild(0),'tag','PROCESSINGSCRIPTMENU')); 
    
    return
    
%%%%%%%%%%%%%%%%%%%%
% SCRIPT FUNCTIONS %
%%%%%%%%%%%%%%%%%%%%


function PreLoopScript

    global SCRIPT SCRIPTDATA;
    
    if length(SCRIPT.GROUPNAME) == 0,
        errordlg('Please define GROUPS before processing the data');
        return
    end
    
    % MAKE OUTPUT DIRECTORIES IF NEEDED
    
    if SCRIPT.TSDFODIRON == 1,
        if ~exist(SCRIPT.TSDFODIR,'dir'),
            mkdir(SCRIPT.TSDFODIR);
        end
    end
    
    if SCRIPT.MATODIRON == 1,
        if ~exist(SCRIPT.MATODIR,'dir'),
            mkdir(SCRIPT.MATODIR);
        end
    end
    
    if SCRIPT.SCIMATODIRON == 1,
        if ~exist(SCRIPT.SCIMATODIR,'dir'),
            mkdir(SCRIPT.SCIMATODIR);
        end
    end
    
    % TELL THE SCRIPT TO START DETECTING THE ALIGNMENT
    % THIS OPTION IS A LITTLE WEIRD AS IT NEEDS A HISTORY OF SIGNALS THAT
    % WERE PROCESSED. SO ON THE FIRST RUN SIGNALS ARE DETECTED AND THEN THE
    % NEXT SIGNALS IN LINE ARE ALIGNED WITH THE PREVIOUS ONES
    SCRIPT.ALIGNSTART = 'detect';
    SCRIPT.ALIGNSIZE = 'detect';
    
    % PREPARE SCRIPT FOR ACQ-FILES
    
    filenames = SCRIPT.ACQFILENAME(SCRIPT.ACQFILES);
    index = [];
    
    for r=1:length(filenames),
        if ~isempty(strfind(filenames{r},'.acq')) || ~isempty(strfind(filenames{r}, '.ac2')), index = [index r]; end
    end
    
    % IF WE HAVE ACQ FILES, THEY NEED MAPPING AND CALIBRATION
    % HENCE THAT IS ACCOMPLISHED HERE
    
    if ~isempty(index),
        
        % THIS PART OF THE CODE NEEDS SOME URGENT REPLACEMENT CURRENTLY IT
        % IS ONLY USED TO DETECT THE ACQ LABEL WHICH IS NEEDED FOR
        % CALIBRATION.
        % I WANT TO REPLACE THIS AND ONLY TAKE FULL NAMES OF THE FILES FOR
        % CALIBRATION
        
        acqfilename = filenames{index(1)};
        
        isac2 = findstr('ac2', acqfilename);
        
        if ~isempty(isac2)
            SCRIPT.ACQLABEL = 'Run';
            SCRIPT.ACQEXT = '.ac2';
        else
            dashpos = findstr('-',acqfilename);
            if ~isempty(dashpos), dashpos = dashpos(end); end

            if ~isempty(dashpos),
                SCRIPT.ACQLABEL = acqfilename(1:(dashpos-1));
            else
                errordlg('cannot decode the ACQ or AC2 filenames');
                error('ERROR')
            end
        end
        
        % THERE ARE ACQ FILES TO BE PROCESSED
        % FIRST CHECK THE EXISTANCE OF THE MAPPINGFILE

        % A MAPPING FILE IS NOT ALWAYS NECESSARY
        % BUT IF ONE IS SUPPLIED IT NEEDS TO EXIST
        mappingfile = SCRIPT.MAPPINGFILE;
        
        if isempty(mappingfile),
            % MAKE SURE mappingfile IS OF STRING TYPE
            % THESE MAKES CODING DOWNSTREAM MORE EASY
            mappingfile = '';
        elseif ~exist(mappingfile,'file'),
            errordlg('The mapping file does not exist');
            error('ERROR')
        end  
    
        % THIS SECTION OF CODE DOES THE CALIBRATION FILE
        % GENERATION
    
        if SCRIPT.DO_CALIBRATE == 1,
 
            if isempty(SCRIPT.CALIBRATIONACQ) & isempty(SCRIPT.CALIBRATIONFILE),
                errordlg('Specify the filenumbers of the calibration measurements');
                error('ERROR');
            end   
		             
            if isempty(SCRIPT.CALIBRATIONFILE),
                SCRIPT.CALIBRATIONFILE = 'calibration.cal8';
            end
             
            calfile = SCRIPT.CALIBRATIONFILE;
            [pn,fn,ext] = fileparts(calfile); 
            if (strcmp(ext,'.cal') ~= 1),
                calfile = fullfile(pn,[fn '.cal8']);
            end
        
		    if ~isempty(SCRIPT.CALIBRATIONACQ),
			    if (~exist(calfile,'file')) | (strcmp(SCRIPT.MAPPINGFILE,SCRIPT.CALIBRATIONMAPPINGUSED) ~= 1) | ( ~isempty(setxor(SCRIPT.CALIBRATIONACQ,SCRIPT.CALIBRATIONACQUSED))), 
				    %for comptability with either ac2 or acq files
                    if ~isempty(strfind(SCRIPT.ACQEXT,'.acq'))
                    acqcalfiles = ioACQFilename(SCRIPT.ACQLABEL,SCRIPT.CALIBRATIONACQ);
                    else
                        acqcalfiles = ioAC2Filename(SCRIPT.ACQLABEL,SCRIPT.CALIBRATIONACQ);
                    end
                    
				    if ~iscell(acqcalfiles), acqcalfiles = {[acqcalfiles]}; end 
				    for p=1:length(acqcalfiles), acqcalfiles{p} = fullfile(SCRIPT.ACQDIR,acqcalfiles{p}); end
				    pointer = get(gcf,'pointer'); set(gcf,'pointer','watch');
				    sigCalibrate8(acqcalfiles{:},mappingfile,calfile,'displaybar');
				    set(gcf,'pointer',pointer);
                    SCRIPT.CALIBRATIONFILE = calfile;
				    SCRIPT.CALIBRATIONACQUSED = SCRIPT.CALIBRATIONACQ;
				    SCRIPT.CALIBRATIONMAPPINGUSED = SCRIPT.MAPPINGFILE;
			    end
		    end 
        end
    end
    
    % RENDER A GLOBAL LIST OF ALL THE BADLEADS
    % THESE LEADS WILL NOT BE USED TO RENDER ANY RMS GRAPHICS
    
    for p=1:length(SCRIPT.GROUPBADLEADS),
        badleads = SCRIPT.GROUPBADLEADS{p};
        if ~isempty(SCRIPT.GROUPBADLEADSFILE{p}),
            bfile = load(SCRIPT.GROUPBADLEADSFILE{p},'-ASCII');
            badleads = union(bfile(:)',badleads);
        end
        
        leads = SCRIPT.GROUPLEADS{p};
        ind = find((badleads>0)&(badleads<=length(leads)));
        badleads = badleads(ind);
        SCRIPT.GBADLEADS{p} = badleads;
    end
    
    % FIND MAXIMUM LEAD
    maxlead = 1;
    for p=1:length(SCRIPT.GROUPLEADS),
        maxlead = max([maxlead SCRIPT.GROUPLEADS{p}]);
    end
    SCRIPT.MAXLEAD = maxlead;
    
    if SCRIPT.DO_INTERPOLATE == 1,
        
        if SCRIPT.DO_SPLIT == 0,
            errordlg('Need to split the signals before interpolation');
            error('ERROR');
        end
        
        for q=1:length(SCRIPT.GROUPNAME),
            SCRIPTDATA.LIBADLEADS{q} = [];   % TRIGGER INITIATION
            SCRIPTDATA.LI{q} = [];
        end
    end
    
    return

function ImportUserSettings(filename,index,fields)

    global SCRIPTDATA TS;
    
    % FIRST FIND THE FILENAME
    
    filenum = strmatch(filename,SCRIPTDATA.FILENAME,'exact');
    
    % THEN RETRIEVE THE DATA FROM THE DATABASE
    
    if ~isempty(filenum),
        for p=1:length(fields),
            if isfield(SCRIPTDATA,fields{p}),
                data = getfield(SCRIPTDATA,fields{p});
                if length(data) >= filenum(1),
                    if ~isempty(data{filenum(1)}),
                        TS{index} = setfield(TS{index},lower(fields{p}),data{filenum(1)});
                    end
                end
            end
        end
    end
    return
    
    
function ExportUserSettings(filename,index,fields)

    global SCRIPTDATA TS;

    % FIRST FIND THE FILENAME
    
    filenum = strmatch(filename,SCRIPTDATA.FILENAME,'exact');

    % IF ENTRY DOES NOT EXIST MAKE ONE
    if isempty(filenum),
        SCRIPTDATA.FILENAME{end+1} = filename;
        filenum = length(SCRIPTDATA.FILENAME);
    end
    
    for p=1:length(fields),
        if isfield(TS{index},lower(fields{p})),
            value = getfield(TS{index},lower(fields{p}));
            if isfield(SCRIPTDATA,fields{p}),
                data = getfield(SCRIPTDATA,fields{p});
            else
                data = {};
            end
            data{filenum(1)} = value;
            SCRIPTDATA = setfield(SCRIPTDATA,fields{p},data);
        else
            value = [];
            if isfield(SCRIPTDATA,fields{p}),
                data = getfield(SCRIPTDATA,fields{p});
            else
                data = {};
            end
            data{filenum(1)} = value;
            SCRIPTDATA = setfield(SCRIPTDATA,fields{p},data);
        end
    end
    SaveScriptData;
    
    return

% THE NEXT FUNCTION IS THE ACTUAL SCRIPT ALL THE ACTION IS IN THIS SCRIPT.
% ANY CHANGES SHOULD BE DONE IN THIS FUNCTION
    
function ProcessACQFile(inputfilename,inputfiledir)

    olddir = pwd;
    global SCRIPT TS SCRIPTDATA;
    
     % CREATE A LIST OF FILES TO BE LOADED:
    
    filename = fullfile(inputfiledir,inputfilename);
    [d1,d2,ext] = fileparts(inputfilename);
    
    files{1} = filename;
    if strcmp(ext,'.acq') || strcmp(ext, '.ac2') == 1,
        
        % IF WE HAVE A MAPPING FILE ADD THE MAPPING FILE
        if ~isempty(SCRIPT.MAPPINGFILE),
            files{end+1} = SCRIPT.MAPPINGFILE;
        end
        
        % IF WE NEED TO CALIBRATE THE ACQ FILE, DO IT!
        if SCRIPT.DO_CALIBRATE == 1,
            if ~isempty(SCRIPT.CALIBRATIONFILE),
                if exist(SCRIPT.CALIBRATIONFILE,'file'),
                    % ADD THE CALIBRATION FILE TO THE LIST OF FILES THAT
                    % NEED TO BE CONSIDERED
                    files{end+1} = SCRIPT.CALIBRATIONFILE;
                end
            end
        end
    end
    
    if strcmp(ext,'.tdsf') == 1,
        if (~isempty(SCRIPT.INPUTTSDFC)),
            files{end+1} = SCRIPT.INPUTTSDFC;    
        end
    end    
        % READ THE ACQ FILE, CALIBRATE IT AND MAP IT 
        
        % THE NEXT FUNCTION READS ACQ, TSDF, AND MAT FILE FORMATS, IT
        % UNDERSTANDS THE MEANING OF .CAL, .CAL8, .TSDFC, .MAPPING AND
        % APPLIES THESE AUTOMATICALLY
        
    if (length(SCRIPT.OPTICALLABEL) > 0) 
        options.opticallabel = SCRIPT.OPTICALLABEL;
        index = ioReadTS(files{:});
    else
        index = ioReadTS(files{:});
    end
    
        % IF THE PACING SIGNAL WAS RECORDED, THIS SIGNAL CAN BE CONVERTED
        % INTO A PACING SEQUENCE TO BE SHOWN AND USED IN THE
        % SIGNALPROCESSING. DETECT THE PEAKS IN THIS SIGNAL IN THE NEXT
        % STEP. THIS OPTION IS SWITCHED ON, AS SOON AS A PACING LEAD IS
        % INDICATED
    
    if size(TS{index}.potvals,1) < SCRIPT.MAXLEAD,
        errordlg('Maximum lead in settings is greater than number of leads in file');
        cd(olddir);
        error('ERROR');
    end
    if ~isempty(SCRIPT.PACINGLEAD) & (SCRIPT.PACINGLEAD > 0) & (SCRIPT.PACINGLEAD <= size(TS{index}.potvals,1)),
         sigDetectPacing(index,SCRIPT.PACINGLEAD);
    
        % CURRENTLY BLANK THE DETECTED FIDUCIALS AS WE WON'T USE THEM RIGHT
        % NOW. THE PACING IS ALSO STORED AS A VECTOR IN PACING, WHICH IS
        % MORE ACCESSIBLE.
        
        TS{index}.fids = [];
        TS{index}.fidset = {};
    end
    
        % LOAD THE USER SETTINGS FROM A PREVIOUS ROUND OF PROCESSING THE
        % DATA. USER DATA INCLUDES:
        % SLICING FRAME/ AVERAGING DATA/ FIDUCIALS/ AVERAGING METHODS /
        % BADLEAD SETTINGS
        % BASICALLY EVERYTHING NEEDED TO PROCESS THE DATA WHICH IS NOT
        % STORED IN THE SCRIPT OR IN THE ACQ FILE
  
    fieldstoload = {'SELFRAMES','AVERAGEMETHOD','AVERAGESTART','AVERAGECHANNEL','AVERAGERMSTYPE','AVERAGEEND','AVERAGEFRAMES','TEMPLATEFRAMES'};
    if SCRIPT.DO_ADDBADLEADS, fieldtoload{end+1} = 'LEADINFO'; end
    ImportUserSettings(inputfilename,index,fieldstoload);
    
        % APPLY THE BAD LEAD SETTINGS, LEADS ALREADY MARKED BAD WILL REMAIN
        % MARKED BAD.
        
    for p=1:length(SCRIPT.GBADLEADS), tsSetBad(index,SCRIPT.GBADLEADS{p}); end

        % DO WE WANT TO DO SOME SLICING AND AVERAGING. IF SO THIS STAGE IS
        % ACTIVATED. THIS IS THE FIRST USER INTERACTION AND LET'S THE USER
        % SELECT A CERTAIN BEAT.
        
    if SCRIPT.DO_FILTER == 1,
        if isfield(SCRIPT,'FILTER'),
            SCRIPT.FILTERSETTINGS = [];
            for p=1:length(SCRIPT.FILTER)
                if strcmp(SCRIPT.FILTER(p).label,SCRIPT.FILTERNAME),
                    SCRIPT.FILTERSETTINGS = SCRIPT.FILTER(p);
                end
            end
            TemporalFilter(index);
        else
            SCRIPT.FILTERSETTINGS.B = [0.03266412226059 0.06320942361376 0.09378788647083 0.10617422096837 0.09378788647083 0.06320942361376 0.03266412226059];
            SCRIPT.FILTERSETTINGS.A = [1];
            
            TemporalFilter(index);
        end
    end
        
        
    if SCRIPT.DO_SLICE == 1,
        
        if SCRIPT.DO_SLICE_USER == 1,
            
            handle = SliceDisplay(index);
            waitfor(handle);
           
            switch SCRIPT.NAVIGATION
                case {'prev','next','stop','back'}, cd(olddir); tsClear(index); return; 
            end
            
            % RELOAD THE BADLEADSETTINGS INTO THE USERINTERFACE
            % REMEMBER BADLEADS ARE RECORDED AS THE NUMBER OF THE LEAD ON
            % THAT PARTICULAR SURFACE. SO WE NEED TO TRANSLATE THEM BACK TO
            % THE LOCAL SYSTEM. HENCE THE intersect COMMAND
            
            if SCRIPT.KEEPBADLEADS == 1,
                badleads = tsIsBad(index);
                for p=1:length(SCRIPT.GROUPBADLEADS), 
                    [dummy,localindex] = intersect(SCRIPT.GROUPLEADS{p},badleads);
                    SCRIPT.GROUPBADLEADS{p} = localindex;
                end
            end
            
            % SO STORE ALL THE SETTINGS/CHANGES WE MADE
            
            ExportUserSettings(inputfilename,index,{'SELFRAMES','AVERAGEMETHOD','AVERAGECHANNEL','AVERAGERMSTYPE','AVERAGESTART','AVERAGEEND','AVERAGEFRAMES','TEMPLATEFRAMES','LEADINFO'});
        end
   
        % CONTINUE AND DO THE SLICING/AVERAGING OPERATION
            
        sigSlice(index);  

    end
    
        % AFTER HAVING MARKED ALL BAD LEADS, WE BLANK THEM AS AN EXTRA
        % OPTION, SO THEY WON'T INTERFERE WITH ANY OTHER SIGNALPROCESSING
        % STEP (OPTIONAL)
        
    if SCRIPT.DO_BLANKBADLEADS == 1,
        badleads = tsIsBad(index);
        TS{index}.potvals(badleads,:) = 0;
        tsSetBlank(index,badleads);
        tsAddAudit(index,'|Blanked out bad leads');
    end
        
    fieldstoload = {'FIDS','FIDSET','STARTFRAME'};
    ImportUserSettings(inputfilename,index,fieldstoload);
    
    if (SCRIPT.DO_BASELINE == 1)|(SCRIPT.DO_DELTAFOVERF == 1),
        
        if ~isfield(TS{index},'startframe'), TS{index}.startframe = 1; end
        
        newstartframe = TS{index}.selframes(1);
        oldstartframe = TS{index}.startframe(1);
          
        fidsShiftFids(index,oldstartframe-newstartframe);    
        TS{index}.startframe = newstartframe;
               
        baseline = fidsFindFids(index,'baseline');
        framelength = size(TS{index}.potvals,2);
        baselinewidth = SCRIPT.BASELINEWIDTH;
            
        % UPDATE BASELINE SETTINGS
        
        if length(baseline) < 2,
            fidsRemoveFiducial(index,'baseline');
            fidsAddFiducial(index,1,'baseline');
            fidsAddFiducial(index,framelength-baselinewidth+1,'baseline');
        end
        
        TS{index}.baselinewidth = baselinewidth;
        if SCRIPT.DO_BASELINE_RMS == 1,
            baselinewidth = SCRIPT.BASELINEWIDTH;
            sigBaseLine(index,[],baselinewidth);
        end
        
        if SCRIPT.DO_BASELINE_USER == 1,
            handle = FidsDisplay(index,2);
            waitfor(handle);
            switch SCRIPT.NAVIGATION
                case {'prev','next','stop','redo','back'}, cd(olddir); tsClear(index); return; 
            end     
        end
        
        ExportUserSettings(inputfilename,index,{'SELFRAMES','AVERAGEMETHOD','AVERAGECHANNEL','AVERAGERMSTYPE','AVERAGESTART','AVERAGEEND','AVERAGEFRAMES','TEMPLATEFRAMES','LEADINFO','FIDS','FIDSET','STARTFRAME'});

        if SCRIPT.DO_DELTAFOVERF == 1,
            if isempty(fidsFindFids(index,20))|isempty(fidsFindFids(index,21)),
                han = errordlg('No interval specified for DetlaF over F correction, skipping correction');
                waitfor(han);
            else            
                sigDeltaFoverF(index);
            end
        end          
        
        if SCRIPT.DO_BASELINE == 1,
            baselinewidth = SCRIPT.BASELINEWIDTH;
            if length(fidsFindFids(index,'baseline')) < 2, 
                han = errordlg('At least two baseline points need to be specified, skipping baseline correction');
                waitfor(han);
            else
                sigBaseLine(index,[],baselinewidth);
            end
        end    
        
  
    end
    
    
    if SCRIPT.DO_LAPLACIAN_INTERPOLATE == 1,
         
        splitgroup = [];
        for p=1:length(SCRIPT.GROUPNAME),
            if SCRIPT.GROUPDONOTPROCESS{p} == 0, splitgroup = [splitgroup p]; end
        end
        
        for q=1:length(splitgroup),
            
            if isempty(SCRIPT.GBADLEADS{splitgroup(q)}),
                continue;
            end
                
            if ~isfield(SCRIPTDATA,'LIBADLEADS'),
                SCRIPTDATA.LIBADLEADS{q} = [];
            end
            
            if ~isempty(setdiff(SCRIPT.GBADLEADS{splitgroup(q)},SCRIPTDATA.LIBADLEADS{q})),
                SCRIPTDATA.LI{splitgroup(q)} = [];
                
                if SCRIPT.GROUPDONOTPROCESS{splitgroup(q)} == 1,
                    continue;
                end
                
                files = {};
                files{1} = SCRIPT.GROUPGEOM{splitgroup(q)};
                if isempty(files{1}),
                    continue;
                end
                if ~isempty(SCRIPT.GROUPCHANNELS{splitgroup(q)}),
                    files{2} = SCRIPT.GROUPCHANNELS{splitgroup(q)};
                end

                SCRIPTDATA.LI{splitgroup(q)} = sparse(triLaplacianInterpolation(files{:},SCRIPT.GBADLEADS{splitgroup(q)},length(SCRIPT.GROUPLEADS{splitgroup(q)})));
            end
            
            if isempty(SCRIPTDATA.LI{splitgroup(q)}),
                continue;
            end
            
            leads = SCRIPT.GROUPLEADS{splitgroup(q)};
            
            TS{index}.potvals(leads,:) = SCRIPTDATA.LI{splitgroup(q)}*TS{index}.potvals(leads,:);
            tsSetInterp(index,leads(SCRIPT.GBADLEADS{splitgroup(q)}));
            
        end
        tsAddAudit(index,'|Interpolated bad leads (Laplacian interpolation)');
    end
    
    
    fieldstoload = {'FIDS','FIDSET','STARTFRAME'};
    ImportUserSettings(inputfilename,index,fieldstoload);
    
    if SCRIPT.DO_DETECT == 1,
        
        if ~isfield(TS{index},'startframe'), TS{index}.startframe = 1; end
        
        newstartframe = TS{index}.selframes(1);
        oldstartframe = TS{index}.startframe(1);
          
        fidsShiftFids(index,oldstartframe-newstartframe);    
        TS{index}.startframe = newstartframe;
               
        baseline = fidsFindFids(index,'baseline');
        framelength = size(TS{index}.potvals,2);
        baselinewidth = SCRIPT.BASELINEWIDTH;
            
        if length(baseline) < 2,
            fidsRemoveFiducial(index,'baseline');
            fidsAddFiducial(index,1,'baseline');
            fidsAddFiducial(index,framelength-baselinewidth+1,'baseline');
        end
        
        
        if SCRIPT.DO_DETECT_USER == 1,
            handle = FidsDisplay(index);
            waitfor(handle);
            switch SCRIPT.NAVIGATION
                case {'prev','next','stop','redo','back'}, cd(olddir); tsClear(index); return; 
            end     
        end
        
        ExportUserSettings(inputfilename,index,{'SELFRAMES','AVERAGEMETHOD','AVERAGERMSTYPE','AVERAGECHANNEL','AVERAGESTART','AVERAGEEND','AVERAGEFRAMES','TEMPLATEFRAMES','LEADINFO','FIDS','FIDSET','STARTFRAME'});
    end
    
    if SCRIPT.DO_SPLIT == 1,
        splitgroup = [];
        for p=1:length(SCRIPT.GROUPNAME),
            if SCRIPT.GROUPDONOTPROCESS{p} == 0, splitgroup = [splitgroup p]; end
        end
        indices = tsSplitTS(index,SCRIPT.GROUPLEADS(splitgroup),ioUpdateFilename('.tsdf',inputfilename,SCRIPT.GROUPEXTENSION(splitgroup)));
        tsDeal(indices,'tsdfcfilename',SCRIPT.GROUPTSDFC(splitgroup));
 
        % THE FILENAMING CONVENTIONS WERE NOT PROPERLY DESIGNED
        % THEY WILL BE CHANGED, HOWEVER TO GARANTEE A STABLE FILENAME
        % OUTPUT, THE FILENAME WILL BE PUT HERE IN THE FILENAME FIELD
        % THE EXTRAFIELD OF NEWFILEEXT WILL BE CLEARED AND IS NOT USED       
        
        tsDeal(indices,'filename',ioUpdateFilename('.tsdf',inputfilename,SCRIPT.GROUPEXTENSION(splitgroup))); 
        tsSet(indices,'newfileext','');
        tsClear(index);
        index = indices;
    end

    
    if SCRIPT.DO_INTERPOLATE == 1,
        if SCRIPT.DO_SPLIT == 0,
            error('Need to split the signal before interpolating');
        end
        for q=1:length(index),
            
            if isempty(SCRIPT.GBADLEADS{splitgroup(q)}),
                continue;
            end
                
            if ~isempty(setdiff(SCRIPT.GBADLEADS{splitgroup(q)},SCRIPTDATA.LIBADLEADS{q})),
                SCRIPTDATA.LI{splitgroup(q)} = [];
                
                if SCRIPT.GROUPDONOTPROCESS{splitgroup(q)} == 1,
                    continue;
                end
                
                files = {};
                files{1} = SCRIPT.GROUPGEOM{splitgroup(q)};
                if isempty(files{1}),
                    continue;
                end
                if ~isempty(SCRIPT.GROUPCHANNELS{splitgroup(q)}),
                    files{2} = SCRIPT.GROUPCHANNELS{splitgroup(q)};
                end

                SCRIPTDATA.LI{splitgroup(q)} = sparse(triLaplacianInterpolation(files{:},SCRIPT.GBADLEADS{splitgroup(q)},length(SCRIPT.GROUPLEADS{splitgroup(q)})));
            end
            
            if isempty(SCRIPTDATA.LI{splitgroup(q)}),
                continue;
            end
            
            TS{index(q)}.potvals = SCRIPTDATA.LI{splitgroup(q)}*TS{index(q)}.potvals;
            tsSetInterp(index(q),SCRIPT.GBADLEADS{splitgroup(q)});
            tsAddAudit(index(q),'|Interpolated bad leads (Laplacian interpolation)');
            
        end
    end
    
    if (SCRIPT.TSDFODIRON == 1),
        olddir = cd(SCRIPT.TSDFODIR);
        tsDeal(index,'filename',ioUpdateFilename('.tsdf',inputfilename,SCRIPT.GROUPEXTENSION(splitgroup)));
        ioWriteTS(index,'noprompt','oworiginal');
        cd(olddir);
    end
    
    if (SCRIPT.MATODIRON == 1),
        olddir = cd(SCRIPT.MATODIR);
        tsDeal(index,'filename',ioUpdateFilename('.mat',inputfilename,SCRIPT.GROUPEXTENSION(splitgroup)));
        ioWriteTS(index,'noprompt','oworiginal');
        cd(olddir);
    end
    
    if (SCRIPT.SCIMATODIRON == 1),
        olddir = cd(SCRIPT.SCIMATODIR);
        tsDeal(index,'filename',ioUpdateFilename('.mat',inputfilename,SCRIPT.GROUPEXTENSION(splitgroup)));
        options = []; options.scirunmat = 1;
        ioWriteTS(index,'noprompt','oworiginal',options);
        cd(olddir);
    end
    
    
    if SCRIPT.DO_INTEGRALMAPS == 1,
        if SCRIPT.DO_DETECT == 0,
            error('Need fiducials to do integralmaps');
        end
        mapindices = fidsIntAll(index);
        % THE FILENAMING CONVENTIONS WERE NOT PROPERLY DESIGNED
        % THEY WILL BE CHANGED, HOWEVER TO GARANTEE A STABLE FILENAME
        % OUTPUT, THE FILENAME WILL BE PUT HERE IN THE FILENAME FIELD
        % THE EXTRAFIELD OF NEWFILEEXT WILL BE CLEARED AND IS NOT USED
        % THIS CONSTRUCTION ALLOWS MORE FREEDOM RIGHT NOW.
        % NEED TO UPGRADE THE FILENAMING SYSTEM!!!
        
        if (SCRIPT.TSDFODIRON==1),
            olddir = cd(SCRIPT.TSDFODIR); 
            tsDeal(mapindices,'filename',ioUpdateFilename('.tsdf',inputfilename,SCRIPT.GROUPEXTENSION(splitgroup),'-itg')); 
            tsSet(mapindices,'newfileext','');
            ioWriteTS(mapindices,'noprompt','oworiginal');
            cd(olddir);
        end
        
        if (SCRIPT.MATODIRON==1),
            olddir = cd(SCRIPT.MATODIR); 
            tsDeal(mapindices,'filename',ioUpdateFilename('.mat',inputfilename,SCRIPT.GROUPEXTENSION(splitgroup),'-itg')); 
            tsSet(mapindices,'newfileext','');
            ioWriteTS(mapindices,'noprompt','oworiginal');
            cd(olddir);
        end
        
        if (SCRIPT.SCIMATODIRON==1),
            olddir = cd(SCRIPT.SCIMATODIR); 
            tsDeal(mapindices,'filename',ioUpdateFilename('.mat',inputfilename,SCRIPT.GROUPEXTENSION(splitgroup),'-itg')); 
            tsSet(mapindices,'newfileext','');
            options = []; options.scirunmat = 1;
            ioWriteTS(mapindices,'noprompt','oworiginal',options);
            cd(olddir);
        end
        
        tsClear(mapindices);
    end
    
   if SCRIPT.DO_ACTIVATIONMAPS == 1,
        if SCRIPT.DO_DETECT == 0,
            error('Need fiducials to do activation maps');
        end
        mapindices = sigActRecMap(index);
        % THE FILENAMING CONVENTIONS WERE NOT PROPERLY DESIGNED
        % THEY WILL BE CHANGED, HOWEVER TO GARANTEE A STABLE FILENAME
        % OUTPUT, THE FILENAME WILL BE PUT HERE IN THE FILENAME FIELD
        % THE EXTRAFIELD OF NEWFILEEXT WILL BE CLEARED AND IS NOT USED
        % THIS CONSTRUCTION ALLOWS MORE FREEDOM RIGHT NOW.
        % NEED TO UPGRADE THE FILENAMING SYSTEM!!!
   
        if (SCRIPT.TSDFODIRON == 1),
            olddir =  cd(SCRIPT.TSDFODIR);
            tsDeal(mapindices,'filename',ioUpdateFilename('.tsdf',inputfilename,SCRIPT.GROUPEXTENSION(splitgroup),'-ari')); 
            tsSet(mapindices,'newfileext','');
            ioWriteTS(mapindices,'noprompt','oworiginal');
            cd(olddir);
        end
        
        if (SCRIPT.MATODIRON == 1),
            olddir = cd(SCRIPT.MATODIR);
            tsDeal(mapindices,'filename',ioUpdateFilename('.mat',inputfilename,SCRIPT.GROUPEXTENSION(splitgroup),'-ari')); 
            tsSet(mapindices,'newfileext','');
            ioWriteTS(mapindices,'noprompt','oworiginal');
            cd(olddir);
        end
        
        if (SCRIPT.SCIMATODIRON == 1),
            olddir = cd(SCRIPT.SCIMATDIR);
            tsDeal(mapindices,'filename',ioUpdateFilename('.mat',inputfilename,SCRIPT.GROUPEXTENSION(splitgroup),'-ari')); 
            tsSet(mapindices,'newfileext','');
            options = []; options.scirunmat = 1;
            ioWriteTS(mapindices,'noprompt','oworiginal',options);
            cd(olddir);
        end
        
        tsClear(mapindices);
        s = tsNew(length(index));
        
        for j=1:length(index),    
            acttime = floor(fidsFindLocalFids(index(j),'act'));
            acttime = round(median([ones(size(acttime)) acttime  (TS{index(j)}.numframes-1)*ones(size(acttime))],2));
            for p=1:TS{index(j)}.numleads,
%                keyboard
                if ~isempty(acttime),
                    %dvdt(p) = (TS{index(j)}.potvals(p,acttime(p)+1) - TS{index(j)}.potvals(p,acttime(p)))/TS{index(j)}.samplefrequency;    
                else
                    %dvdt(p) = 0;
                end
            end
           % TS{index(j)}.dvdt = dvdt;
            TS{s(j)} = TS{index(j)};
            %TS{s(j)}.potvals = dvdt;
            TS{s(j)}.numframes = 1;
            TS{s(j)}.pacing = [];
            TS{s(j)}.fids = [];
            TS{s(j)}.fidset = {};
            TS{s(j)}.audit = '| Dv/Dt at activation';
            
        end

%         if (SCRIPT.TSDFODIRON == 1),
%             olddir =  cd(SCRIPT.TSDFODIR);
%             tsDeal(s,'filename',ioUpdateFilename('.tsdf',inputfilename,SCRIPT.GROUPEXTENSION(splitgroup),'-dvdt')); 
%             tsSet(s,'newfileext','');
%             ioWriteTS(s,'noprompt','oworiginal');
%             cd(olddir);
%         end
% 
%         if (SCRIPT.MATODIRON == 1),
%             olddir = cd(SCRIPT.MATODIR);
%             tsDeal(s,'filename',ioUpdateFilename('.mat',inputfilename,SCRIPT.GROUPEXTENSION(splitgroup),'-dvdt')); 
%             tsSet(s,'newfileext','');
%             ioWriteTS(s,'noprompt','oworiginal');
%             cd(olddir);
%         end
% 
%         if (SCRIPT.SCIMATODIRON == 1),
%             olddir = cd(SCRIPT.SCIMATDIR);
%             tsDeal(s,'filename',ioUpdateFilename('.mat',inputfilename,SCRIPT.GROUPEXTENSION(splitgroup),'-dvdt')); 
%             tsSet(s,'newfileext','');
%             options = []; options.scirunmat = 1;
%             ioWriteTS(s,'noprompt','oworiginal',options);
%             cd(olddir);
%         end

        tsClear(s);

   end
   
    SaveScriptData;
    SaveSettings([]);
    tsClear(index);

    return
    
    
    