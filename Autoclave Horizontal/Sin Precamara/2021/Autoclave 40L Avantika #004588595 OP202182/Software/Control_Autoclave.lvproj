<?xml version='1.0' encoding='UTF-8'?>
<Project Type="Project" LVVersion="11008008">
	<Property Name="CCSymbols" Type="Str"></Property>
	<Property Name="NI.LV.All.SourceOnly" Type="Bool">false</Property>
	<Property Name="NI.Project.Description" Type="Str"></Property>
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
		<Item Name="Control_Autoclave.vi" Type="VI" URL="../Control_Autoclave.vi"/>
		<Item Name="Dependencies" Type="Dependencies">
			<Item Name="BulkUSB_MN (subvi).vi" Type="VI" URL="../BulkUSB_MN (subvi).vi"/>
			<Item Name="mpusbapi.dll" Type="Document" URL="/C/pic32_solutions/USB Tools/MCHPUSB Custom Driver/Mpusbapi/Dll/Borland_C/mpusbapi.dll"/>
		</Item>
		<Item Name="Build Specifications" Type="Build">
			<Item Name="Control_Autoclave" Type="EXE">
				<Property Name="App_copyErrors" Type="Bool">true</Property>
				<Property Name="App_INI_aliasGUID" Type="Str">{ECECE3DE-F156-48B5-865A-E6B3C5B7DBB3}</Property>
				<Property Name="App_INI_GUID" Type="Str">{C71E6DD3-05B3-47E6-B13D-449A325C65B3}</Property>
				<Property Name="Bld_buildCacheID" Type="Str">{92E34497-7582-4F2F-BC26-2AA79DFC09A6}</Property>
				<Property Name="Bld_buildSpecDescription" Type="Str">Interfaz para Autoclave Digital</Property>
				<Property Name="Bld_buildSpecName" Type="Str">Control_Autoclave</Property>
				<Property Name="Bld_excludeLibraryItems" Type="Bool">true</Property>
				<Property Name="Bld_excludePolymorphicVIs" Type="Bool">true</Property>
				<Property Name="Bld_localDestDir" Type="Path">../builds/NI_AB_PROJECTNAME/Control_Autoclave</Property>
				<Property Name="Bld_localDestDirType" Type="Str">relativeToCommon</Property>
				<Property Name="Bld_modifyLibraryFile" Type="Bool">true</Property>
				<Property Name="Bld_previewCacheID" Type="Str">{B0328BBF-FC93-47E6-B253-7C32B967D4A1}</Property>
				<Property Name="Destination[0].destName" Type="Str">AutoclaveJP.exe</Property>
				<Property Name="Destination[0].path" Type="Path">../builds/NI_AB_PROJECTNAME/Control_Autoclave/AutoclaveJP.exe</Property>
				<Property Name="Destination[0].preserveHierarchy" Type="Bool">true</Property>
				<Property Name="Destination[0].type" Type="Str">App</Property>
				<Property Name="Destination[1].destName" Type="Str">Support Directory</Property>
				<Property Name="Destination[1].path" Type="Path">../builds/NI_AB_PROJECTNAME/Control_Autoclave/data</Property>
				<Property Name="DestinationCount" Type="Int">2</Property>
				<Property Name="Source[0].itemID" Type="Str">{83B6D87C-EAFA-478D-A056-3370EDA1EDE9}</Property>
				<Property Name="Source[0].type" Type="Str">Container</Property>
				<Property Name="Source[1].destinationIndex" Type="Int">0</Property>
				<Property Name="Source[1].itemID" Type="Ref">/My Computer/Control_Autoclave.vi</Property>
				<Property Name="Source[1].sourceInclusion" Type="Str">TopLevel</Property>
				<Property Name="Source[1].type" Type="Str">VI</Property>
				<Property Name="SourceCount" Type="Int">2</Property>
				<Property Name="TgtF_fileDescription" Type="Str">Control_Autoclave</Property>
				<Property Name="TgtF_fileVersion.major" Type="Int">1</Property>
				<Property Name="TgtF_internalName" Type="Str">Control_Autoclave</Property>
				<Property Name="TgtF_legalCopyright" Type="Str">Copyright © 2014 </Property>
				<Property Name="TgtF_productName" Type="Str">Control_Autoclave</Property>
				<Property Name="TgtF_targetfileGUID" Type="Str">{9E582504-DF7E-4E13-AD6A-DD1C60B95933}</Property>
				<Property Name="TgtF_targetfileName" Type="Str">AutoclaveJP.exe</Property>
			</Item>
		</Item>
	</Item>
</Project>
