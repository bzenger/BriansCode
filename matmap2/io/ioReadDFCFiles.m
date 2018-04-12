function TSDFCarray = ioReadDFCFiles(varargin)

% FUNCTION TSDFCarray = ioReadDFCFiles(filenames)
%
% DESCRIPTION
% This function reads the pointers to the TSDFC-files which are stored in the DFC-files. If DFC files are nested multiple
% files are read until the level where everything is a TSDFC-file. This function will return the TSDFC entries found in 
% a DFC-file. The function differs from ioReadTSDFCFiles in that respect that it returns TSDFC-files and NOT TSDF-files. 
% This function was designed to support the reading of DFC-files as if they are arrays of TSDFC-files. Hence it is used 
% in preparsing the TSDFC/DFC-file arrays to consist only of TSDFC-files. In the reading process DFC-files are interpreted
% as being an array of TSDFC-files.
%
% INPUT
% filenames     The DFC-file names. This can be a string or string array.
%
% OUTPUT
% tdsfcarray    The TDSFC-files contained in the DFC-file(s).
%
% SEE ALSO ioReadTSDFCFiles

% JG Stinstra 2002

% Do the processing of the input, which can be span multiple inputs

filenames = {};
for p=1:nargin,
    if ischar(varargin{p}),
         filenames{end+1} = varargin{p};
    end
    if iscellstr(varargin{p}),
         filenames(end+1:end+length(varargin{p})) = varargin{p};	 
    end
end

% define an empty output array
TSDFCarray = {};

for p=1:length(filenames),
    array = mexListDFC(filenames{p}); % Go to mex and get the file listing
    for q=1:length(array),
	[pn,fn,ext] = fileparts(array{q});
	switch ext,
	case '.dfc'
	    newarray = ioReadDFCFiles(array{q}); % go back and resolve this dfc first    
	    TSDFCarray(end+1:end+length(newarray)) = newarray;
    	case '.tsdfc' 
	    TSDFCarray{end+1} = array{q};
	end
    end
end

% This should have done the trick, the array should now contain all TSDFC-files which are connected
% to the DFC-file. From here it is disassembling the TSDFC files.

TSDFCarray = sort(TSDFCarray); % assure a consistent order

return    	