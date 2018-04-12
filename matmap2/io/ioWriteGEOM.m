function ioWriteGEOM(varargin)

% FUNCTION ioWriteGEOM(filename,geomdata,['noprompt'])
%
% DESCRIPTION
% This function writes a .geom file
%
% INPUT
%  filename       the filename(s) for saving the data (append extension .geom)
%  geomdata       indices to the GEOM structure or direct structured data
% 
% OPTIONS
%  noprompt       do not prompt before overwriting a file
%
% OUTPUT
%  -
%
% GEOM STRUCTURE
%  .filename      - filename
%  .name          - the name of the data (where it came from)
%  .pts           - pts file
%  .cpts          - the conductivity tensors per point
%  .fac           - fac file
%  .cfac          - the conductivity tensors per element
%  .channels      - the channels file
%
% SEE ALSO ioReadGEOM, ioReadGEOMdata

ioWriteGEOMdata(varargin{:});

