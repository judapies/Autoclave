<?xml version='1.0' encoding='UTF-8'?>
<Project Type="Project" LVVersion="11008008">
	<Item Name="My Computer" Type="My Computer">
		<Property Name="server.app.propertiesEnabled" Type="Bool">true</Property>
		<Property Name="server.control.propertiesEnabled" Type="Bool">true</Property>
		<Property Name="server.tcp.enabled" Type="Bool">false</Property>
		<Property Name="server.tcp.port" Type="Int">0</Property>
		<Property Name="server.tcp.serviceName" Type="Str">My Computer/VI Server</Property>
		<Property Name="server.tcp.serviceName.default" Type="Str">My Computer/VI Server</Property>
		<Property Name="server.vi.callsEnabled" Type="Bool">true</Property>
		<Property Name="server.vi.propertiesEnabled" Type="Bool">true</Property>
		<Property Name="specify.custom.address" Type="Bool">false</Property>
		<Item Name="Autoclave.ico" Type="Document" URL="/C/Users/maria consuelo/Downloads/Autoclave.ico"/>
		<Item Name="Graficas_Autoclave.vi" Type="VI" URL="../Graficas_Autoclave.vi"/>
		<Item Name="Dependencies" Type="Dependencies">
			<Item Name="BulkUSB_MN (subvi).vi" Type="VI" URL="../BulkUSB_MN (subvi).vi"/>
			<Item Name="mpusbapi.dll" Type="Document" URL="/C/pic32_solutions/USB Tools/MCHPUSB Custom Driver/Mpusbapi/Dll/Borland_C/mpusbapi.dll"/>
		</Item>
		<Item Name="Build Specifications" Type="Build">
			<Item Name="Graficas_Autoclave" Type="EXE">
				<Property Name="App_copyErrors" Type="Bool">true</Property>
				<Property Name="App_INI_aliasGUID" Type="Str">{8327F168-D6E4-48C9-8ACC-E448BA111D2E}</Property>
				<Property Name="App_INI_GUID" Type="Str">{66CD9C03-EE7A-4EFC-A68C-E0F3230C4D13}</Property>
				<Property Name="Bld_buildCacheID" Type="Str">{F76DD2B6-84B9-4CD0-BE3A-EE41AB0280CA}</Property>
				<Property Name="Bld_buildSpecName" Type="Str">Graficas_Autoclave</Property>
				<Property Name="Bld_excludeLibraryItems" Type="Bool">true</Property>
				<Property Name="Bld_excludePolymorphicVIs" Type="Bool">true</Property>
				<Property Name="Bld_localDestDir" Type="Path">../builds/NI_AB_PROJECTNAME/Graficas_Autoclave</Property>
				<Property Name="Bld_localDestDirType" Type="Str">relativeToCommon</Property>
				<Property Name="Bld_modifyLibraryFile" Type="Bool">true</Property>
				<Property Name="Bld_previewCacheID" Type="Str">{A2297759-BD52-4954-BB01-01C17F5FEA3F}</Property>
				<Property Name="Destination[0].destName" Type="Str">AutoclaveViewer.exe</Property>
				<Property Name="Destination[0].path" Type="Path">../builds/NI_AB_PROJECTNAME/Graficas_Autoclave/AutoclaveViewer.exe</Property>
				<Property Name="Destination[0].preserveHierarchy" Type="Bool">true</Property>
				<Property Name="Destination[0].type" Type="Str">App</Property>
				<Property Name="Destination[1].destName" Type="Str">Support Directory</Property>
				<Property Name="Destination[1].path" Type="Path">../builds/NI_AB_PROJECTNAME/Graficas_Autoclave/data</Property>
				<Property Name="DestinationCount" Type="Int">2</Property>
				<Property Name="Exe_iconItemID" Type="Ref">/My Computer/Autoclave.ico</Property>
				<Property Name="Source[0].itemID" Type="Str">{73E3DE0E-6B3A-41AA-8058-D8AC765CEB62}</Property>
				<Property Name="Source[0].type" Type="Str">Container</Property>
				<Property Name="Source[1].destinationIndex" Type="Int">0</Property>
				<Property Name="Source[1].itemID" Type="Ref">/My Computer/Graficas_Autoclave.vi</Property>
				<Property Name="Source[1].sourceInclusion" Type="Str">TopLevel</Property>
				<Property Name="Source[1].type" Type="Str">VI</Property>
				<Property Name="SourceCount" Type="Int">2</Property>
				<Property Name="TgtF_companyName" Type="Str">JP Bioingenieria SAS</Property>
				<Property Name="TgtF_fileDescription" Type="Str">Graficas_Autoclave</Property>
				<Property Name="TgtF_fileVersion.major" Type="Int">2</Property>
				<Property Name="TgtF_fileVersion.minor" Type="Int">1</Property>
				<Property Name="TgtF_internalName" Type="Str">Graficas_Autoclave</Property>
				<Property Name="TgtF_legalCopyright" Type="Str">Copyright © 2016 JP Bioingenieria SAS</Property>
				<Property Name="TgtF_productName" Type="Str">Graficas_Autoclave</Property>
				<Property Name="TgtF_targetfileGUID" Type="Str">{C6F86B71-3DE2-4CDD-AD2D-F2A9B351C9B0}</Property>
				<Property Name="TgtF_targetfileName" Type="Str">AutoclaveViewer.exe</Property>
			</Item>
			<Item Name="Instalador_Autoclave" Type="Installer">
				<Property Name="Destination[0].name" Type="Str">Graficas_Autoclave2</Property>
				<Property Name="Destination[0].parent" Type="Str">{3912416A-D2E5-411B-AFEE-B63654D690C0}</Property>
				<Property Name="Destination[0].tag" Type="Str">{47BB59DA-2692-4236-BBFD-B079AAD81878}</Property>
				<Property Name="Destination[0].type" Type="Str">userFolder</Property>
				<Property Name="DestinationCount" Type="Int">1</Property>
				<Property Name="DistPart[0].flavorID" Type="Str">DefaultFull</Property>
				<Property Name="DistPart[0].productID" Type="Str">{066F687E-1CA0-4D94-A2C9-F8E6E817F4CB}</Property>
				<Property Name="DistPart[0].productName" Type="Str">NI LabVIEW Run-Time Engine 2011</Property>
				<Property Name="DistPart[0].upgradeCode" Type="Str">{6AD6C111-233B-4924-A3BF-E4813FE23C7A}</Property>
				<Property Name="DistPartCount" Type="Int">1</Property>
				<Property Name="INST_author" Type="Str">Hewlett-Packard Company</Property>
				<Property Name="INST_autoIncrement" Type="Bool">true</Property>
				<Property Name="INST_buildLocation" Type="Path">../builds/Graficas_Autoclave/Instalador_Autoclave</Property>
				<Property Name="INST_buildLocation.type" Type="Str">relativeToCommon</Property>
				<Property Name="INST_buildSpecName" Type="Str">Instalador_Autoclave</Property>
				<Property Name="INST_defaultDir" Type="Str">{47BB59DA-2692-4236-BBFD-B079AAD81878}</Property>
				<Property Name="INST_productName" Type="Str">Graficas_Autoclave</Property>
				<Property Name="INST_productVersion" Type="Str">1.0.1</Property>
				<Property Name="InstSpecBitness" Type="Str">32-bit</Property>
				<Property Name="InstSpecVersion" Type="Str">11008029</Property>
				<Property Name="MSI_arpCompany" Type="Str">JP Bioingenieria SAS</Property>
				<Property Name="MSI_distID" Type="Str">{5A827FF0-6B99-486D-A1E1-291D6778ABFA}</Property>
				<Property Name="MSI_osCheck" Type="Int">0</Property>
				<Property Name="MSI_upgradeCode" Type="Str">{8FAECC77-1074-4E15-A83B-ADEDC1FFC285}</Property>
				<Property Name="RegDest[0].dirName" Type="Str">Software</Property>
				<Property Name="RegDest[0].dirTag" Type="Str">{DDFAFC8B-E728-4AC8-96DE-B920EBB97A86}</Property>
				<Property Name="RegDest[0].parentTag" Type="Str">2</Property>
				<Property Name="RegDestCount" Type="Int">1</Property>
				<Property Name="Source[0].dest" Type="Str">{47BB59DA-2692-4236-BBFD-B079AAD81878}</Property>
				<Property Name="Source[0].name" Type="Str">Graficas_Autoclave.vi</Property>
				<Property Name="Source[0].tag" Type="Ref">/My Computer/Graficas_Autoclave.vi</Property>
				<Property Name="Source[0].type" Type="Str">File</Property>
				<Property Name="Source[1].dest" Type="Str">{47BB59DA-2692-4236-BBFD-B079AAD81878}</Property>
				<Property Name="Source[1].File[0].dest" Type="Str">{47BB59DA-2692-4236-BBFD-B079AAD81878}</Property>
				<Property Name="Source[1].File[0].name" Type="Str">AutoclaveViewer.exe</Property>
				<Property Name="Source[1].File[0].Shortcut[0].destIndex" Type="Int">0</Property>
				<Property Name="Source[1].File[0].Shortcut[0].name" Type="Str">AutoclaveViewer</Property>
				<Property Name="Source[1].File[0].Shortcut[0].subDir" Type="Str">Graficas_Autoclave2</Property>
				<Property Name="Source[1].File[0].ShortcutCount" Type="Int">1</Property>
				<Property Name="Source[1].File[0].tag" Type="Str">{C6F86B71-3DE2-4CDD-AD2D-F2A9B351C9B0}</Property>
				<Property Name="Source[1].FileCount" Type="Int">1</Property>
				<Property Name="Source[1].name" Type="Str">Graficas_Autoclave</Property>
				<Property Name="Source[1].tag" Type="Ref">/My Computer/Build Specifications/Graficas_Autoclave</Property>
				<Property Name="Source[1].type" Type="Str">EXE</Property>
				<Property Name="SourceCount" Type="Int">2</Property>
			</Item>
		</Item>
	</Item>
</Project>
