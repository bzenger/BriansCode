function Pts  = ioReadPts(filename)

% FUNCTION pts  = ioReadPts(filename)
%
% DESCRIPTION
% This function reads a pts-geometry file into a matrix
%
% INPUT
% filename      name of the pts-file
%
% OUTPUT
% pts           matrix containing the positions of nodes
%
% SEE ALSO ioWritePts, ioReadFac

% First ensure that the file has the correct extension

[pn,fn,ext] = fileparts(filename);

% Try to correct a faulty file name
if strcmp(ext,'pts') == 0, % no match
    filename = fullfile(pn,[fn '.pts']);
end

FID = fopen(filename,'r');

if FID < 1,
    err = sprintf('Could not open file : %s\n',filename);
    msgError(err,3);
end

Pts = fscanf(FID,'%f',[3,inf]); % scan all numbers

fclose(FID);

return