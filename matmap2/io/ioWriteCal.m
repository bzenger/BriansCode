function result = ioWriteCal(filename,cal)

% FUNCTION ioWriteCal(filename,cal)
%
% DESCRIPTION
% This function writes a .cal file.
%
% INPUT
% filename   The filename to be used for the .cal file
%            The extension .cal is automatically added
% cal        The vector containing the calibrations
%
% OUTPUT
% -
%
% SEE ALSO ioReadCal

    [pn,fn,ext] = fileparts(filename);
    filename = fullfile(pn,[fn '.cal']);
    
    FID = fopen(filename,'w');
    
    if FID < 1,
       err = sprintf('Could not open file : %s\n',filename);
       msgError(err,3);
       result = 0;
       return
    end
    
    fprintf(FID,'%d\n0\n%d\n',length(cal),length(cal));
    fprintf(FID,'%7.8f\n',cal);
    
    fclose(FID);
    