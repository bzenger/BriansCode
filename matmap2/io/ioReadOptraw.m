function index = ioReadOptraw(varargin)
% FUNCTION TSindex = ioReadOptraw(filename)
%
% DESCRIPTION
% Internal function for reading optical data
%
% INPUT
% filename      What is the filename
%
% OUTPUT -
%
% SEE ALSO ioReadTS

[files,TSindex,options] = ioiInputParameters(varargin);

if length(files.optraw) > 1,
    msgError('This function only processes one optraw-file',3);
end

%%
% Obtain the tsdffilename as key for the container file
% Specifying a tsdffile overrules the name given in the TS-structure
    
global TS;    
numts = 1;


optfile = files.optraw{1};

if (~exist(optfile,'file')),
    msgError(sprintf('File : %s does not exist',optfile),3);
    return;
end

index = tsNew(1);
FID = fopen(optfile,'r','b');

if isstruct(options)
    if isfield(options,'opticallabel'),
        num = str2num(optfile(4:end));
        TS{index}.filename = sprintf('%s-%03d.opt',options.opticallabel,num);
    else
        TS{index}.filename = optfile;   
    end
else
    TS{index}.filename = optfile;
end

readfile = 1;
if isfield(options,'scantsdffile')
    if options.scantsdffile == 1, readfile = 0;
    end
end
        
if readfile,
    TS{index}.potvals = fread(FID,[4096 inf],'ushort=>double');
else
    TS{index}.potvals = zeros(4096,1);
end
fclose(FID);


TS{index}.label = sprintf('optical data');
TS{index}.numframes = size(TS{index}.potvals,2);
TS{index}.numleads = size(TS{index}.potvals,1);
TS{index}.leadinfo = zeros(TS{index}.numleads,1);
TS{index}.text = '';
TS{index}.expid = '';
TS{index}.unit = 'mV';
TS{index}.audit = sprintf('|Original data file:%s ',optfile);
TS{index}.samplefrequency = 960;

D = dir(optfile);
TS{index}.time = D.date;

return
    
