program prjDelphiSystem;

uses
  System.StartUpCopy,
  FMX.Forms,
  untPortariaApi in 'untPortariaApi.pas',
  untPrincipalDS in 'untPrincipalDS.pas' {Form2},
  ObservabilityHeartbeat in 'ObservabilityHeartbeat.pas';

{$R *.res}

begin
  Application.Initialize;
  Application.CreateForm(TForm2, Form2);
  Application.Run;
end.
