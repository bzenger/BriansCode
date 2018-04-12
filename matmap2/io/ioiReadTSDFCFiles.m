function TSDFarray = ioReadTSDFCFiles(varargin)

% FUNCTION TSDFarray = ioReadTSDFCFiles(filenames,options)
%
% DESCRIPTION
% This function reads the file-pointers contained in a TSDFC-file or a DFC-file. The function
% returns the filenames in the order of the TSDFC-files. Within a TSDFC-file the keys are
% alphabetically sorted by the function. If a DFC-file is supplied this file will be translated into 
% TSDFC-filenames first and subsequently they will be processed as an array of TSDFC-files.
% The function is intented to get all the TSDF-filenames to be loaded. Since the default option
% of ioReadTS in case no TSDF-filename is supplied is to read them all, this function is called
% establish what is all.
%
% INPUT
% filenames      A string or cell-array with strings specifying the TSDFC/DFC-files to be processed
%
% OUTPUT
% TSDFarray      The TDSF-files contained in the DFC/TSDFC-files.
%
% SEE ALSO ioReadDFCFiles

% JG Stinstra 2002

% Do the processing of the input, which can be span multiple inputs

[files,dummy,options] = ioiInputParameters(varargin); % parse the input directly to the main input translating function

filenames = files.tsdfc; % get the tsdfc files which should have been passed
 

% define an empty output array
TSDFarray = {};

for p=1:length(filenames),
    array = mexListDFC(filenames{p}); % Go to mex and get the file listing
    TSDFarray(end+1:end+length(array)) = sort(array); % append array at the end
end

return    	