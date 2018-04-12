function TSindices = ioiReadTSDF(varargin)

% FUNCTION TSindices = ioiReadTSDF(tsdffilename,[indices],[options])
%
% INTERNAL FUNCTION
% Avoid using this function, as it might change in the future
% Use ioReadTS instead
%
% DESCRIPTION
% This function reads one TSDF file into the TS-structure. The function is based on
% mexReadTSDF, but has some additional functionality, for instance it adds data on where
% the data originated from, so when using ioWriteTSDF it knows what filenames to use by
% default. 
% 
% INPUT
% tsdffilename          A single tsdf-filename for reading
% indices               In case multiple timeseries are stored, select the ones needed
% options               For future usage
%
% OUTPUT
% TSindices             Indices into the TS-structure where the data has been loaded
%
% SEE ALSO ioReadTS, ioWriteTSDF, ioWriteTS

%%
% This function represents the matlab side of processing the data
% 1) the function reads the tsdf-file specified
% 2) It checks for the number of timeseries in the file
% 3) It creates a new TS structure
% 4) It checks the AUDIT string whether any additional data has been stored in there
% 

[files,indices,options] = ioiInputParameters(varargin);

global TS;

tsdffilename = files.tsdf{1};
TSindices = [];

if isempty(tsdffilename),
    msgError('No TSDF-file specified',3);
    return;
end    

% Depending on whether a set of indices is specified all timeseries will be read
% or only the indicated ones

if isempty(indices),
    [tsdffilename,tsnumber] = utilStripNumber(tsdffilename);
    if ~isempty(tsnumber),								% We are reading just one timeseries specified by @<number>
        labels = mexScanTSDF(tsdffilename);
        if tsnumber > length(labels),			
            msgError('You tried to access a timeseries number that is not in the file',1); return
        end   
        TSindices = tsNew(1);
        TS(TSindices) = mexReadTSDF(tsdffilename,tsnumber,options);
    else
        labels = mexScanTSDF(tsdffilename);						% scan the file to see how many entries there are
        numtimeseries = length(labels);							% How many to read. Normally it should just be one
        if isfield(options,'timeseries'),						
            numtimeseries = length(options.timeseries);					% User already decided which ones he wanted
        end
        TSindices = tsNew(numtimeseries);						% get the same number of new entries 
        TS(TSindices) = mexReadTSDF(tsdffilename,options);				% load the timeseries into memory
    end    
else
    TSindices = tsNew(length(indices));
    TS(TSindices) = mexReadTSDF(tsdffilename,indices,options);
end    

%%
% Add some more fields
% Since the sample frequency is not recorded in the data, matlab adds this datafield
% in the audit string. So here I scan whether I can retrieve  this data field.
% If not I assume the default samplefrequency. Lateron the user can change the field
% if he/she knows the actual one
% add as well a field to store an additional filename extesion, since we are not overwriting
% old files this option can be used to put an extension depending on the computation the 
% program did. This will help you to standardise names 


for p = 1:length(TSindices),
    audit = TS{TSindices(p)}.audit;
 
    sampstr = 'TSDF_SAMPLEFREQUENCY=';   
    index = findstr(audit,sampstr);
    if ~isempty(index)
        index=index(1)+strlen(sampstr);
        TS{TSindices(p)}.samplefrequency = sscanf('%dHz',audit(index:index+12));
    else 
        TS{TSindices(p)}.samplefrequency = 1000;
    end        

    TS{TSindices(p)}.newfileext = ''; % extension for the new filename
end    

return