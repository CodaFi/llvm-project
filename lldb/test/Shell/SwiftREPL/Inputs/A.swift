public class Foo {
  public static func foo() -> String  {
    print("A")
    return "A"
  }
}

public struct MyPoint {
  let x: Int
  let y: Int


  public init(x: Int, y: Int) {
    self.x = x
    self.y = y
  }

  public var magnitudeSquared: Int {
    return x*x + y*y
  }
}

public struct FromInterface {}
public struct OtherType {}

public let testValue = FromInterface()

extension StringProtocol {
  public func foo() -> Bool { true }
}
