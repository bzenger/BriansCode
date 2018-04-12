function ioWriteTSDFC(tsdfcfile,key,TSindex)
% FUNCTION ioWriteTSDFC(tsdfcfile,key,TSindex)
%
% INTERNAL FUNCTION
% Try to avoid direct usage of this function. Use ioWriteTS instead
%
% DESCRIPTION
% This function writes an entry in the TSDFC-file. At the moment the function only
% supports fiducials, but in the future it should be able to do a rewrite. This still
% requires some programming.
%
% INPUT
% tsdfcfile                the tsdfc file to store the data in
% key                      the name of the tsdf file.
% TSindex                  the index from where to get the fiducials
%
% OUTPUT -
%
% SEE ALSO -

global TS;

% The function is pretty short as most work is done in the mexfile
% all data is checked again in the mex-file and new labels are generated
% by this function as well. So the only thing to do here is to verify
% whether there are fiducials and get them from the global.

if nargin ~= 3,
    msgError('Three inputs are required',3);
    return
end    

if isempty(tsdfcfile),
    return;
end

if isempty(key),
    return;
end    

if isfield(TS{TSindex},'fids'),
    fids = TS{TSindex}.fids;
    if isfield(TS{TSindex},'fidset'),
        fidset = TS{TSindex}.fidset;
    else
        fidset = {};
    end
    if size(fids,2) == 0, return; end
    mexWriteTSDFC(tsdfcfile,key,fids,fidset);
else
    mexWriteTSDFC(tsdfcfile,key);
end    

return    