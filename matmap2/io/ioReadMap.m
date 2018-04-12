function map = ioReadmap(filename)
% FUNCTION map = ioReadmap(filename)
%
% DESCRIPTION
% This function loads a mapping/channels file. A mapping file is directly loaded
% into memory as it specifies where each channel should be located. A channels file
% is translated to be in the same format of the mapping file if possible. The
% structure of the latter file is different and thence needs a small conversion to
% act as a remapping file. In a mapping file each subsequent lead in the resulting 
% data matrix is assigned a new set of data whereas for a channels file this is not
% necessarily true. Hence in the latter case zeros are put in between.
% 
% INPUT
% filename		name of the file (wildcards allowed)
%
% OUTPUT
% map			a vector specifying which channels should be mapped to position one,two
%                       etc. map(1) is the new lead/frame at position one, map(2) for position two
%                       and so on.
%
% SEE ALSO ioReadChannels, ioReadMapping

map =[];							% default output

if nargin ~= 1,							% does the user obeys the rules
    msgError('You should specify a filename',5);
    return
end    

newfilename = utilExpandFilenames(filename);			% Do wildcard thing
if length(newfilename) > 1,					% But we support only one file
    msgError('You should specify only one filename',3);
    return;
else
    if ~isempty(newfilename),
        filename = newfilename{1};					% make it a string again as utilExpandFilenames made a cell array
    end
end    

[pn,fn,ext] = fileparts(filename);
switch ext,
case {'.mapping','.mux'}
    map = ioReadMapping(filename);
case {'.channels'}
    channels = ioReadChannels(filename);
    map = zeros(1,max(channels(:,1)));		% Do the translation process
    map(channels(:,1)) = channels(:,2);		% generate a mapping vector 

otherwise
    
    filename = fullfile(pn,[fn '.channels']);
    if exist(filename,'file')
        channels = ioReadChannels(filename);
        map = zeros(1,max(channels(:,1)));		% Do the translation process
        map(channels(:,1)) = channels(:,2);		% generate a mapping vector 
    else    
        warning('You did not specify a filename containing mapping information');
        map = [];
    end
end

return
