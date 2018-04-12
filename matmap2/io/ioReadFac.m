function Fac  = ioReadFac(filename)

% FUNCTION fac  = ioReadFac(filename)
%
% DESCRIPTION
% This function reads a fac-geometry file into a matrix
%
% INPUT
% filename      name of the FAC-file
%
% OUTPUT
% fac           matrix containing the triangulation information
%
% SEE ALSO ioWriteFac

% First ensure that the file has the correct extension

[pn,fn,ext] = fileparts(filename);

% Try to correct a faulty file name
if strcmp(ext,'fac') == 0, % no match
    filename = fullfile(pn,[fn '.fac']);
end

FID = fopen(filename,'r');

if FID < 1,
    err = sprintf('Could not open file : %s\n',filename);
    msgError(err,3);
end

Fac = fscanf(FID,'%d',[3,inf]); % scan all numbers

fclose(FID);

return