using System;
using System.Collections.Generic;
using LibreHardwareMonitor.Hardware;

namespace TrafficMonitorTests
{
    public class FakeSensor : ISensor
    {
        private readonly float? value;
        public FakeSensor(SensorType type, string name, float? reading) { SensorType = type; Name = name; value = reading; }
        public IControl Control { get { return null; } }
        public IHardware Hardware { get { return null; } }
        public Identifier Identifier { get { return null; } }
        public int Index { get { return 0; } }
        public bool IsDefaultHidden { get { return false; } }
        public float? Max { get { return value; } }
        public float? Min { get { return value; } }
        public string Name { get; set; }
        public IReadOnlyList<IParameter> Parameters { get { return new IParameter[0]; } }
        public SensorType SensorType { get; private set; }
        public float? Value { get { return value; } }
        public IEnumerable<SensorValue> Values { get { return new SensorValue[0]; } }
        public TimeSpan ValuesTimeWindow { get; set; }
        public void ResetMin() { }
        public void ResetMax() { }
        public void ClearValues() { }
        public void Accept(IVisitor visitor) { visitor.VisitSensor(this); }
        public void Traverse(IVisitor visitor) { }
    }
    public class FakeHardware : IHardware
    {
        public FakeHardware(ISensor[] sensors) { Sensors = sensors; }
        public HardwareType HardwareType { get { return HardwareType.Cpu; } }
        public Identifier Identifier { get { return null; } }
        public string Name { get; set; }
        public IHardware Parent { get { return null; } }
        public ISensor[] Sensors { get; private set; }
        public IHardware[] SubHardware { get { return new IHardware[0]; } }
        public IDictionary<string, string> Properties { get { return new Dictionary<string, string>(); } }
        public event SensorEventHandler SensorAdded { add { } remove { } }
        public event SensorEventHandler SensorRemoved { add { } remove { } }
        public string GetReport() { return "Test fixture"; }
        public void Update() { }
        public void Accept(IVisitor visitor) { visitor.VisitHardware(this); }
        public void Traverse(IVisitor visitor) { }
    }
}
