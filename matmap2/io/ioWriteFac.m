function result = ioWriteFac(filename,Fac)

% function result = ioWriteFac(filename,Fac)
%
% this function writes a fac-geometry file to file
%
% INPUT
%  filename - name of the output file
%  Fac      - matrix containing the triangulation data
%
% OUTPUT
%  result   - is one in case of success or zero in case of failure


% Test whether Fac needs to be transposed

if size(Fac,1) ~= 3,
    Fac = Fac';
    if size(Fac,1) ~= 3,
	msgError('Fac does not have proper dimensions',4);
        result = 0;
        return
    end
end

% First ensure that the file has the correct extension

[pn,fn,ext] = fileparts(filename);

% Try to correct a faulty file name
if strcmp(ext,'fac') == 0, % no match
    filename = fullfile(pn,[fn '.fac']);
end

FID = fopen(filename,'w');

if FID < 1,
    err = sprintf('Could not open file : %s\n',filename);
    msgError(err,3);
    result = 0;
    return
end

fprintf(FID,'%5.0d %5.0d %5.0d\n',Fac);
fclose(FID);

result = 1; % everything went OK
return
