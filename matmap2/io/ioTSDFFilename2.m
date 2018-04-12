function name = ioTSDFFilename(varargin)
% FUNCTION filename = ioTSDFFilename(label,filenumber,[filenameaddons,...])
%
% DESCRIPTION
% This function generates the TSDF filenames from the different pieces
%
% INPUT
% label           the label of the series
% filenumber      the number of the file or files (in case of more numbers it 
%                 creates a cell array of filenames)
% filenamesaddons addons like '_itg' or '_epi' etc.
%
% OUTPUT
% filename        the filename or a list of filenames (cellarray)
%
% SEE ALSO ioTSDFFilename


name  = '';
for p = 1:nargin,
    if ischar(varargin{p}),
        if iscell(name),
            for q=1:length(name), name{q} = [name{q} varargin{p}]; end
        else
            name = [name varargin{p}];
        end
    elseif isnumeric(varargin{p})
        if iscell(name)
            error('Only one numeric argument allowed');
        else
            if length(varargin{p}) > 1,
                for q=1:length(varargin{p}),
                    namecell{q} = [name sprintf('-%04d',varargin{p}(q))];
                end
                name = namecell;
            else
                name = [name sprintf('-%04d',varargin{p})];
            end
        end
    end
end

if iscell(name)
    for q=1:length(name),
        name{q} = [name{q} '.tsdf'];
    end
else     
    name = [name '.tsdf'];
end

return
